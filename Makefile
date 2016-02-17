include $(TOPDIR)/rules.mk

PKG_NAME:=uploadwpa
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)


include $(INCLUDE_DIR)/package.mk



define Package/uploadwpa
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=Uploads a WPA handshake to various online crackers!
	DEPENDS:=+libstdcpp +libc
endef

define Package/uploadwpa/description
	Uploads a WPA handshake to various online crackers!
endef


define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef


define Package/uploadwpa/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/uploadwpa $(1)/bin/
endef


$(eval $(call BuildPackage,uploadwpa))
