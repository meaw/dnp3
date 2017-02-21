lcov-reset:
	@rm -Rf lcov
	@find . -iname "*.gcda" -exec rm {} \;
	@lcov --directory . --zerocounters

clean-lcov:
	@find . -iname "*.gcno" -exec rm {} \;

if LCOV
lcov-report:
	@rm -Rf lcov
	@mkdir -p lcov
	@touch lcov/all.out
	lcov --compat-libtool --directory src --base-directory $(srcdir) --capture --output-file lcov/all.out
	genhtml -o lcov/html lcov/all.out

lcov: lcov-report

else
lcov-report lcov:
	@echo ""
	@echo "Package not configured with --enable-lcov support!  Please re-run:"
	@echo ""
	@echo "   ./configure --enable-lcov"
	@echo "   make clean"
	@echo "   make lcov"
	@echo ""
endif

