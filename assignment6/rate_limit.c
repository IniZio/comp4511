/* file: flow_net.c 
    Drop packets based on Flow */

/* Step 1: Headers and module setup */
#include <asm/param.h>
#include <linux/jiffies.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Author's name");
MODULE_DESCRIPTION("Drop packets based on Flow");

/* Step 2: Module parameters and hook operation data structure */
static int port = 80;
MODULE_PARM_DESC(port, "Port number. The default port is 80");
module_param(port, int, 0644);

static int pcount = 1000;
MODULE_PARM_DESC(pcount, "Maximum packets allowed before dropping. The default value is 1000");
module_param(pcount, int, 0644);

static int rate = 2000;
MODULE_PARM_DESC(rate, "Ratelimitin bytes/ms. Defaultis 2000");
module_param(rate, int, 0644);

// static int curr_total = 0;
// static unsigned long start = jiffies;
// static unsigned long timeout = jiffies + HZ; // 1 second timeout
// static unsigned long old_bytecount = 0;

static struct nf_hook_ops nfho;

/* Step 3 - Flow struct definition */
struct Flow {
    unsigned int local_ip;
    unsigned int remote_ip;
    unsigned short int local_port;
    unsigned short int remote_port;
    // statistics
    unsigned int pkt_count, drop_count, byte_count, byte_drop_count;

    int initialized;
    unsigned long prev_jiffies;
    unsigned long delta_ms;
};
// FLOW_SIZE = 1<<12 = 4096
#define FLOW_SIZE (1<<12) 

// Create "FLOW_SIZE" buckets to record the flows
static struct Flow flow_list[FLOW_SIZE]; 


/* Step 4: Supporting functions implementation */

// Define a simple hash function for flows
static inline unsigned int hash(struct Flow *f) 
{
    // We can apply any suitable hash algorithm
    // The return value is always in the range of 0..FLOW_SIZE-1
    return ( (f->local_ip%FLOW_SIZE+1) * (f->remote_ip%FLOW_SIZE+1) * 
        ( f->local_port%FLOW_SIZE+1) * (f->remote_port%FLOW_SIZE+1) )% FLOW_SIZE;
}

// An initialization function for a flow
static inline void reset_flow(struct Flow *f) 
{
    if ( f ) { // reset everything to 0
        f->local_port = f->remote_port = 0; 
        f->local_ip = f->remote_ip = 0;     
        f->pkt_count = f->drop_count = 0;   
        f->byte_count = f->byte_drop_count = 0;

        if (!f->initialized) {
            f->initialized = 1;
            f->prev_jiffies = jiffies;
            f->delta_ms = 0;
//            f->timeout = jiffies + HZ; // 1 second timeout
        }
    }
}

/* Step 5: The hook function implementation */
static unsigned int hook_func(void *priv, 
                            struct sk_buff *skb, 
                            const struct nf_hook_state *state)
{
    // Define local variables
    struct iphdr *iph;                  // IP header structure
    struct tcphdr *tcph;                // TCP header structure
    unsigned int sip, dip;              // IP addresses
    unsigned int sport, dport;          // Source and Destination port
    struct Flow f;                      // a local flow struct
    struct Flow *fp;                    // a pointer to a flow
    unsigned int payload_len;           // TCP payload length
   
    iph = ip_hdr(skb);                  // Retrieve IP header from skb

    if ( iph->protocol == IPPROTO_TCP ) {
        tcph = tcp_hdr(skb);            // Retrieve TCP header from skb
        sip = iph->saddr;
        dip = iph->daddr;
        sport = ntohs(tcph->source);
        dport = ntohs(tcph->dest);
        if ( sport != port && dport != port )
            return NF_ACCEPT;

        // Calculate the payload length
        payload_len = ntohs(iph->tot_len)-(iph->ihl<<2)-(tcph->doff<<2);

        // Reset a local flow struct
        reset_flow(&f);
        f.local_ip = sip;
        f.remote_ip = dip;
        f.local_port = sport;
        f.remote_port = dport;

        // Use the hash function to find the flow index
        fp = &flow_list[ hash(&f) ];

        if ( tcph->fin )
            reset_flow(fp); // end of TCP connection, reset flow
        
        if ( payload_len <= 0)
            return NF_ACCEPT;   // not a data packet, accept and don't do counting

        int exceeded_rate_limit = 0;

        fp->delta_ms += ((time_after(jiffies, fp->prev_jiffies) ? jiffies - fp->prev_jiffies : sizeof (unsigned long) - fp->prev_jiffies + jiffies) / HZ) * 1000;

        // printk("[Delta ms] %d and %d\n", fp->delta_ms, (fp->byte_count + payload_len) / fp->delta_ms);
        fp->prev_jiffies = jiffies;

        exceeded_rate_limit = (fp->byte_count/* + payload_len*/) / fp->delta_ms > rate;

        // Drop/Accept the data packet
        if ( exceeded_rate_limit ) {
            fp->drop_count++;
            fp->byte_drop_count += payload_len;

            // printk(KERN_INFO "[FLOW-DROP] [%pI4:%d->%pI4:%d] payload:%d  pkts(accept/drop)  %d:%d  bytes(accept/drop) %d:%d\n",
            //             &f.local_ip, ntohs(f.local_port),
            //             &f.remote_ip, ntohs(f.remote_port),
            //             payload_len, fp->pkt_count, fp->drop_count,
            //             fp->byte_count, fp->byte_drop_count);
            printk("[Finish rate = %d] t = %d ms, receive / drop(bytes) : %d/%d\n", rate, fp->delta_ms, fp->byte_count, fp->byte_drop_count);
            return NF_DROP;
        } else {
            fp->pkt_count++;
            fp->byte_count += payload_len;

            // printk(KERN_INFO "[FLOW-ACCEPT] [%pI4:%d->%pI4:%d] payload:%d  pkts(accept/drop)  %d:%d  bytes(accept/drop) %d:%d\n",
            //             &f.local_ip, ntohs(f.local_port),
            //             &f.remote_ip, ntohs(f.remote_port),
            //             payload_len, fp->pkt_count, fp->drop_count,
            //             fp->byte_count, fp->byte_drop_count);
            printk("[Finish rate = %d] t = %d ms, receive / drop(bytes) : %d/%d\n", rate, fp->delta_ms, fp->byte_count, fp->byte_drop_count);
            return NF_ACCEPT;   
        }
        
    }
    return NF_ACCEPT; // Accept all other cases
}

int flow_net_init_module(void)
{
    int i;
    for (i=0; i<FLOW_SIZE; i++)
        reset_flow(&flow_list[i]);      // init flows

    nfho.hook = hook_func;              // hook function
    nfho.hooknum = NF_INET_PRE_ROUTING; // pre-routing
    nfho.pf = PF_INET;                  // IPV4 packets
    nfho.priority = NF_IP_PRI_FIRST;	// set to the highest priority

    nf_register_net_hook(&init_net, &nfho); // register hook
    printk(KERN_INFO "[Init] Flow Netfilter Module\n");
    return 0;
}

void flow_net_exit_module(void)
{
    nf_unregister_net_hook(&init_net, &nfho);
    printk(KERN_INFO "[Exit] Flow Netfilter Module\n");
}

module_init(flow_net_init_module);
module_exit(flow_net_exit_module);
