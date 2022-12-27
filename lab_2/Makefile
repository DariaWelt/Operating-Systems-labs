CFLAGS = -std=c++11 -Wall -Werror
PFLAGS = -pthread -lrt

CONNECTIONS = $(shell find ./connection -name '*.cpp')

all:
	@$(foreach connection,$(wildcard $(CONNECTIONS)),$(call build,$(connection:./connection/conn_%.cpp=%)))

define build
    echo $(1) ;
	g++ $(CFLAGS) ./host/* ./connection/*.h ./connection/conn_$(1).cpp -o host_$(1) $(PFLAGS) ;
	echo host_$(1) ;
endef

clean:
	@rm -f host_*
