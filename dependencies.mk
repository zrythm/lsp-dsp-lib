# Variables that describe dependencies
LSP_COMMON_LIB_VERSION     := 1.0.7
LSP_COMMON_LIB_NAME        := lsp-common-lib
LSP_COMMON_LIB_TYPE        := src
LSP_COMMON_LIB_URL         := https://github.com/sadko4u/$(LSP_COMMON_LIB_NAME).git

LSP_TEST_FW_VERSION        := 1.0.5
LSP_TEST_FW_NAME           := lsp-test-fw
LSP_TEST_FW_TYPE           := src
LSP_TEST_FW_URL            := https://github.com/sadko4u/$(LSP_TEST_FW_NAME).git

STDLIB_VERSION             := system
STDLIB_TYPE                := opt
STDLIB_LDFLAGS             := -lpthread

ifeq ($(PLATFORM),Windows)
  TEST_STDLIB_VERSION        := system
  TEST_STDLIB_TYPE           := opt
  TEST_STDLIB_LDFLAGS        := -lshlwapi
else
  TEST_STDLIB_VERSION        := system
  TEST_STDLIB_TYPE           := opt
  TEST_STDLIB_LDFLAGS        := 
endif
