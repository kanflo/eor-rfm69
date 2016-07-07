# Component makefile for extras/rfm69

INC_DIRS += $(rfm69_ROOT)

# args for passing into compile rule generation
rfm69_SRC_DIR =  $(rfm69_ROOT)

$(eval $(call component_compile_rules,rfm69))

