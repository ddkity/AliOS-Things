NAME := id2_client

$(NAME)_SOURCES     := id2_client.c id2_client_demo.c

$(NAME)_COMPONENTS  := id2 irot alicrypto

LINUXHOST_ID2_CFLAGS	= -DID2_OTP_SUPPORTED=1 -DID2_CRYPTO_TYPE_CONFIG=ID2_CRYPTO_TYPE_AES -DID2_OTP_LOCAL_TEST=1 -DID2_DEBUG=1
 KM_ID2_CFLAGS			= -DID2_OTP_SUPPORTED=1 -DID2_CRYPTO_TYPE_CONFIG=ID2_CRYPTO_TYPE_AES
TEE_ID2_CFLAGS			= -DID2_OTP_SUPPORTED=0 -DID2_CRYPTO_TYPE_CONFIG=ID2_CRYPTO_TYPE_AES
 SE_ID2_CFLAGS			= -DID2_OTP_SUPPORTED=0 -DID2_CRYPTO_TYPE_CONFIG=ID2_CRYPTO_TYPE_3DES

ifeq ($(findstring linuxhost, $(BUILD_STRING)), linuxhost)
	$(NAME)_CFLAGS      += $(LINUXHOST_ID2_CFLAGS)
else ifeq ($(findstring mk3060, $(BUILD_STRING)), mk3060)
	$(NAME)_CFLAGS      += $(KM_ID2_CFLAGS)
else ifeq ($(findstring mk3080, $(BUILD_STRING)), mk3080)
	$(NAME)_CFLAGS      += $(KM_ID2_CFLAGS)
else ifeq ($(findstring cb2201, $(BUILD_STRING)), cb2201)
	$(NAME)_CFLAGS      += $(TEE_ID2_CFLAGS)
endif

GLOBAL_INCLUDES += ./