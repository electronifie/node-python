LD_PRELOAD=/usr/lib64/libpython2.7.so
DEBUG=node-python*
LD_PRELOAD=/usr/lib64/libpython2.7.so
PYTHONPATH=./test/support
REPORTER=spec

test:
	$(MAKE) DEBUG= test-debug

test-debug:
	DEBUG=$(DEBUG) \
	LD_PRELOAD=$(LD_PRELOAD) \
	PYTHONPATH=$(PYTHONPATH) \
	./node_modules/.bin/mocha -R $(REPORTER)

test-jenkins:
	$(MAKE) JUNIT_REPORT_PATH=test-report.xml JUNIT_REPORT_STACK=1 REPORTER=mocha-jenkins-reporter test

.PHONY: test test-debug
