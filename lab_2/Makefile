CFLAGS = -std=c++17 -Wall -Werror -I client/ -I conn/ -I host/ -I utils/
PFLAGS = -lpthread -lrt

CONNS = $(shell find ./conn -name 'conn_*.cpp')

all:
	@$(foreach conn,$(wildcard $(CONNS)),$(call build,$(conn:./conn/conn_%.cpp=%)))

define build
	g++ $(CFLAGS) host/*.cpp conn/conn_$(1).cpp -o wolf-and-goats-$(1) $(PFLAGS) ;
endef

clean:
	@rm -f wolf-and-goats-*
