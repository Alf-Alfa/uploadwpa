include $(TOPDIR)/rules.mk

PKG_NAME:=uploadwpa2
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)


include $(INCLUDE_DIR)/package.mk



define Package/uploadwpa2
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=Uploads a WPA handshake to various online crackers Custom sites and SSL Support
	DEPENDS:=+libstdcpp +libc +libcrypto +libopenssl 
endef

define Package/uploadwpa2/description
	Uploads a WPA handshake to various online crackers!
endef


define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef


define Package/uploadwpa2/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/uploadwpa2 $(1)/bin/
endef


$(eval $(call BuildPackage,uploadwpa2))
