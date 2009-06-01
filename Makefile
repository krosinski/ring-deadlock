debug: init.cpp messages.cpp node.cpp detector.cpp
	g++ -Wall -g -o $(PVM_ROOT)/bin/$(PVM_ARCH)/init -I$(PVM_ROOT)/include init.cpp messages.cpp -L$(PVM_ROOT)/lib/$(PVM_ARCH) -lpvm3 -lgpvm3
	g++ -Wall -g -o $(PVM_ROOT)/bin/$(PVM_ARCH)/node -I$(PVM_ROOT)/include node.cpp messages.cpp -L$(PVM_ROOT)/lib/$(PVM_ARCH) -lpvm3 -lgpvm3
	g++ -Wall -g -o $(PVM_ROOT)/bin/$(PVM_ARCH)/detector -I$(PVM_ROOT)/include detector.cpp messages.cpp -L$(PVM_ROOT)/lib/$(PVM_ARCH) -lpvm3

release: init.cpp messages.cpp node.cpp detector.cpp
	g++ -O2 -o $(PVM_ROOT)/bin/$(PVM_ARCH)/init -I$(PVM_ROOT)/include init.cpp messages.cpp -L$(PVM_ROOT)/lib/$(PVM_ARCH) -lpvm3 -lgpvm3
	g++ -O2 -o $(PVM_ROOT)/bin/$(PVM_ARCH)/node -I$(PVM_ROOT)/include node.cpp messages.cpp -L$(PVM_ROOT)/lib/$(PVM_ARCH) -lpvm3 -lgpvm3
	g++ -O2 -o $(PVM_ROOT)/bin/$(PVM_ARCH)/detector -I$(PVM_ROOT)/include detector.cpp messages.cpp -L$(PVM_ROOT)/lib/$(PVM_ARCH) -lpvm3
