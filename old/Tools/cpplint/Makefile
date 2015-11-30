ifneq ("$(SOURCES_VERIFY_CODE_STYLE)","")
    CPP_LINT_PATH=$(ROOT_PATH)/Tools/cpplint

    LINTER_PARAMS=--extensions=hpp,cpp,h,c --filter=-legal/copyright

    PREBUILD_TARGETS += code_style_check

code_style_check:
	echo $(SOURCES_VERIFY_CODE_STYLE) | xargs python2 $(CPP_LINT_PATH)/cpplint.py $(LINTER_PARAMS)
endif

