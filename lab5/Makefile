all: noshm shm1 shm2 shm3_consumer shm3_prod

noshm:
	gcc -o noshm noshm.c

shm1:
	gcc -o shm1 shm1.c

shm2:
	gcc -o shm2 shm2.c

shm3_consumer:
	gcc shm3_consumer.c -lrt -o consumer

shm3_prod:
	gcc shm3_producer.c -lrt -o producer

# clear:
#   rm noshm shm1 shm2 consumer producer
