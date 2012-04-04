
# The assumption is the SDL12.so and TouchControlOverlay.so libs are stored under your home
# directory and you used Momentics IDE to import the git projects. Just modify the path here
# if you've done this without Momentics or it's located in a different directory.

QNX_LIBZ_ROOT=~/git
SDL_LIB=$(QNX_LIBZ_ROOT)/SDL/Device-Release/libSDL12.so
TCO_LIB=$(QNX_LIBZ_ROOT)/TouchControlOverlay/Device-Release/libTouchControlOverlay.so


BBTOOLS=${QNX_HOST}/usr/bin/blackberry-
BBPKG=${BBTOOLS}nativepackager
BBDEPLOY=$(BBTOOLS)deploy
BBTOKEN=$(BBTOOLS)debugtokenrequest
BBSIGNER=$(BBTOOLS)signer

BAR_NAME=unreal_speccy_portable.bar
BAR_XML=bar-descriptor.xml
BIN_TARGET=../Release/unreal_speccy_portable



# For registered developers that wish to deploy the .bar file to all users ...
# It's free to register and fairly straight forward ...

BB_CSJ_PIN=000000
BB_CSJ_PASS=speccy
BB_RDK_CERT=client-RDK-000000000.csj
BB_PBDT_CERT=client-PBDT-00000000.csj
BB_EXTRA=author.t12
BB_DEVICE_IP=169.254.0.1
BB_DEVICE_PASSWORD=qaqa


pkg: libz $(BIN_TARGET)
	$(BBPKG) -devMode -package $(BAR_NAME) $(BAR_XML)  -e $(BIN_TARGET)  unreal_speccy_portable media/zxspectrum.png media/* *.so

install:
	@echo example deploy command to playbook with address 169.254.0.1 and password qwqw
	@echo http://docs.blackberry.com/en/developers/?userType=21  and search for blackberry-deploy for more info
	$(BBDEPLOY) -installApp -device $(BB_DEVICE_IP) -package $(BAR_NAME) -password $(BB_DEVICE_PASSWORD) 

# The following commands are typical next steps for deployment. The signing requires CSJ pin, pass and certificates
# because a request is made directly to RIM to sign the package. The only purpose of this is for deployment on all 
# devices and not your own... 

register:
	@echo  get developer 'keys' https://www.blackberry.com/SignedKeys/
	@echo modify the BB_xxx variables above to the proper values you have obtained ...
	blackberry-signer -verbose -register -storepass $(BB_CSJ_PASS) -csjpin $(BB_CSJ_PIN) $(BB_RDK_CERT) $(BB_PBDT_CERT) $(BB_EXTRA)
sign:
	blackberry-signer -verbose -storepass $(BB_CSJ_PASS)  $(BAR_NAME)

libz:
	@cp $(SDL_LIB) .
	@cp $(TCO_LIB) .
