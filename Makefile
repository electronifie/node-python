DEBUG=node-python*
LD_PRELOAD=/usr/lib64/libpython2.7.so
PYTHONPATH=./test/support

test:
	$(MAKE) DEBUG= test-debug
	
test-debug:
	DEBUG=$(DEBUG) \
	LD_PRELOAD=$(LD_PRELOAD) \
	PYTHONPATH=$(PYTHONPATH) \
	./node_modules/.bin/mocha -R spec

.PHONY: test test-debug
