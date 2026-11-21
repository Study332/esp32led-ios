import os
import shutil

base_dir = r"c:\Users\Hieu-PC\Desktop\esp32led"
ios_dir = os.path.join(base_dir, "ios_app")
src_dir = os.path.join(ios_dir, "ESP32LED")
proj_dir = os.path.join(ios_dir, "ESP32LED.xcodeproj")
github_dir = os.path.join(base_dir, ".github", "workflows")

os.makedirs(src_dir, exist_ok=True)
os.makedirs(proj_dir, exist_ok=True)
os.makedirs(github_dir, exist_ok=True)

# Copy index.html into src_dir
shutil.copy2(os.path.join(base_dir, "index.html"), os.path.join(src_dir, "index.html"))

# 1. Info.plist
info_plist = """<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleDevelopmentRegion</key>
    <string>$(DEVELOPMENT_LANGUAGE)</string>
    <key>CFBundleExecutable</key>
    <string>$(EXECUTABLE_NAME)</string>
    <key>CFBundleIdentifier</key>
    <string>com.hieu.esp32led</string>
    <key>CFBundleInfoDictionaryVersion</key>
    <string>6.0</string>
    <key>CFBundleName</key>
    <string>$(PRODUCT_NAME)</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleShortVersionString</key>
    <string>1.0</string>
    <key>CFBundleVersion</key>
    <string>1</string>
    <key>LSRequiresIPhoneOS</key>
    <true/>
    <key>NSCameraUsageDescription</key>
    <string>Ứng dụng cần quyền Camera để nhận diện cử chỉ bàn tay điều khiển LED ESP32</string>
    <key>NSAppTransportSecurity</key>
    <dict>
        <key>NSAllowsArbitraryLoads</key>
        <true/>
    </dict>
    <key>UIApplicationSceneManifest</key>
    <dict>
        <key>UIApplicationSupportsMultipleScenes</key>
        <false/>
        <key>UISceneConfigurations</key>
        <dict>
            <key>UIWindowSceneSessionRoleApplication</key>
            <array>
                <dict>
                    <key>UISceneConfigurationName</key>
                    <string>Default Configuration</string>
                    <key>UISceneDelegateClassName</key>
                    <string>$(PRODUCT_MODULE_NAME).SceneDelegate</string>
                </dict>
            </array>
        </dict>
    </dict>
    <key>UILaunchStoryboardName</key>
    <string>LaunchScreen</string>
    <key>UIRequiredDeviceCapabilities</key>
    <array>
        <string>armv7</string>
    </array>
    <key>UISupportedInterfaceOrientations</key>
    <array>
        <string>UIInterfaceOrientationPortrait</string>
    </array>
</dict>
</plist>
"""
with open(os.path.join(src_dir, "Info.plist"), "w", encoding="utf-8") as f:
    f.write(info_plist)

# 2. AppDelegate.swift
app_delegate = """import UIKit

@main
class AppDelegate: UIResponder, UIApplicationDelegate {
    func application(_ application: UIApplication, didFinishLaunchingWithOptions launchOptions: [UIApplication.LaunchOptionsKey: Any]?) -> Bool {
        return true
    }

    func application(_ application: UIApplication, configurationForConnecting connectingSceneSession: UISceneSession, options: UIScene.ConnectionOptions) -> UISceneConfiguration {
        return UISceneConfiguration(name: "Default Configuration", sessionRole: connectingSceneSession.role)
    }
}
"""
with open(os.path.join(src_dir, "AppDelegate.swift"), "w", encoding="utf-8") as f:
    f.write(app_delegate)

# 3. SceneDelegate.swift
scene_delegate = """import UIKit

class SceneDelegate: UIResponder, UIWindowSceneDelegate {
    var window: UIWindow?

    func scene(_ scene: UIScene, willConnectTo session: UISceneSession, options connectionOptions: UIScene.ConnectionOptions) {
        guard let windowScene = (scene as? UIWindowScene) else { return }
        window = UIWindow(windowScene: windowScene)
        window?.rootViewController = ViewController()
        window?.makeKeyAndVisible()
    }
}
"""
with open(os.path.join(src_dir, "SceneDelegate.swift"), "w", encoding="utf-8") as f:
    f.write(scene_delegate)

# 4. ViewController.swift
view_controller = """import UIKit
import WebKit

class ViewController: UIViewController, WKUIDelegate, WKNavigationDelegate {
    var webView: WKWebView!

    override func viewDidLoad() {
        super.viewDidLoad()
        view.backgroundColor = UIColor(red: 15/255, green: 23/255, blue: 42/255, alpha: 1.0)
        
        setupWebView()
        loadWebContent()
    }

    private func setupWebView() {
        let config = WKWebViewConfiguration()
        config.allowsInlineMediaPlayback = true
        config.mediaTypesRequiringUserActionForPlayback = []
        if #available(iOS 14.0, *) {
            config.defaultWebpagePreferences.allowsContentJavaScript = true
        }

        webView = WKWebView(frame: view.bounds, configuration: config)
        webView.autoresizingMask = [.flexibleWidth, .flexibleHeight]
        webView.uiDelegate = self
        webView.navigationDelegate = self
        webView.isOpaque = false
        webView.backgroundColor = .clear
        webView.scrollView.bounces = false
        
        view.addSubview(webView)
    }

    private func loadWebContent() {
        if let htmlPath = Bundle.main.path(forResource: "index", ofType: "html") {
            let fileURL = URL(fileURLWithPath: htmlPath)
            webView.loadFileURL(fileURL, allowingReadAccessTo: fileURL.deletingLastPathComponent())
        } else {
            print("Lỗi: Không tìm thấy file index.html trong Bundle")
        }
    }

    // Cấp quyền truy cập Camera cho WKWebView trên iOS 15+
    @available(iOS 15.0, *)
    func webView(_ webView: WKWebView, requestMediaCapturePermissionFor origin: WKSecurityOrigin, initiatedByFrame frame: WKFrameInfo, type: WKMediaCaptureType, decisionHandler: @escaping (WKPermissionDecision) -> Void) {
        decisionHandler(.grant)
    }

    override var preferredStatusBarStyle: UIStatusBarStyle {
        return .lightContent
    }
}
"""
with open(os.path.join(src_dir, "ViewController.swift"), "w", encoding="utf-8") as f:
    f.write(view_controller)

# 5. project.pbxproj
pbxproj = """// !$*UTF8*$!
{
	archiveVersion = 1;
	classes = {
	};
	objectVersion = 54;
	objects = {

/* Begin PBXBuildFile section */
		A1000001 /* AppDelegate.swift in Sources */ = {isa = PBXBuildFile; fileRef = B1000001 /* AppDelegate.swift */; };
		A1000002 /* SceneDelegate.swift in Sources */ = {isa = PBXBuildFile; fileRef = B1000002 /* SceneDelegate.swift */; };
		A1000003 /* ViewController.swift in Sources */ = {isa = PBXBuildFile; fileRef = B1000003 /* ViewController.swift */; };
		A1000004 /* index.html in Resources */ = {isa = PBXBuildFile; fileRef = B1000004 /* index.html */; };
/* End PBXBuildFile section */

/* Begin PBXFileReference section */
		B1000000 /* ESP32LED.app */ = {isa = PBXFileReference; explicitFileType = wrapper.application; includeInIndex = 0; path = ESP32LED.app; sourceTree = BUILT_PRODUCTS_DIR; };
		B1000001 /* AppDelegate.swift */ = {isa = PBXFileReference; lastKnownFileType = sourcecode.swift; path = AppDelegate.swift; sourceTree = "<group>"; };
		B1000002 /* SceneDelegate.swift */ = {isa = PBXFileReference; lastKnownFileType = sourcecode.swift; path = SceneDelegate.swift; sourceTree = "<group>"; };
		B1000003 /* ViewController.swift */ = {isa = PBXFileReference; lastKnownFileType = sourcecode.swift; path = ViewController.swift; sourceTree = "<group>"; };
		B1000004 /* index.html */ = {isa = PBXFileReference; lastKnownFileType = text.html; path = index.html; sourceTree = "<group>"; };
		B1000005 /* Info.plist */ = {isa = PBXFileReference; lastKnownFileType = text.plist.xml; path = Info.plist; sourceTree = "<group>"; };
/* End PBXFileReference section */

/* Begin PBXFrameworksBuildPhase section */
		C1000001 /* Frameworks */ = {
			isa = PBXFrameworksBuildPhase;
			buildActionMask = 2147483647;
			files = (
			);
			runOnlyForDeploymentPostprocessing = 0;
		};
/* End PBXFrameworksBuildPhase section */

/* Begin PBXGroup section */
		D1000001 = {
			isa = PBXGroup;
			children = (
				D1000002 /* ESP32LED */,
				D1000003 /* Products */,
			);
			sourceTree = "<group>";
		};
		D1000002 /* ESP32LED */ = {
			isa = PBXGroup;
			children = (
				B1000001 /* AppDelegate.swift */,
				B1000002 /* SceneDelegate.swift */,
				B1000003 /* ViewController.swift */,
				B1000004 /* index.html */,
				B1000005 /* Info.plist */,
			);
			path = ESP32LED;
			sourceTree = "<group>";
		};
		D1000003 /* Products */ = {
			isa = PBXGroup;
			children = (
				B1000000 /* ESP32LED.app */,
			);
			name = Products;
			sourceTree = "<group>";
		};
/* End PBXGroup section */

/* Begin PBXNativeTarget section */
		E1000001 /* ESP32LED */ = {
			isa = PBXNativeTarget;
			buildConfigurationList = F1000001 /* Build configuration list for PBXNativeTarget "ESP32LED" */;
			buildPhases = (
				C1000002 /* Sources */,
				C1000001 /* Frameworks */,
				C1000003 /* Resources */,
			);
			buildRules = (
			);
			dependencies = (
			);
			name = ESP32LED;
			productName = ESP32LED;
			productReference = B1000000 /* ESP32LED.app */;
			productType = "com.apple.product-type.application";
		};
/* End PBXNativeTarget section */

/* Begin PBXProject section */
		F1000000 /* Project object */ = {
			isa = PBXProject;
			attributes = {
				BuildIndependentTargetsInParallel = 1;
				LastSwiftUpdateCheck = 1500;
				LastUpgradeCheck = 1500;
			};
			buildConfigurationList = F1000002 /* Build configuration list for PBXProject "ESP32LED" */;
			compatibilityVersion = "Xcode 14.0";
			developmentRegion = en;
			hasScannedForEncodings = 0;
			knownRegions = (
				en,
				Base,
			);
			mainGroup = D1000001;
			productRefGroup = D1000003 /* Products */;
			projectDirPath = "";
			projectRoot = "";
			targets = (
				E1000001 /* ESP32LED */,
			);
		};
/* End PBXProject section */

/* Begin PBXResourcesBuildPhase section */
		C1000003 /* Resources */ = {
			isa = PBXResourcesBuildPhase;
			buildActionMask = 2147483647;
			files = (
				A1000004 /* index.html in Resources */,
			);
			runOnlyForDeploymentPostprocessing = 0;
		};
/* End PBXResourcesBuildPhase section */

/* Begin PBXSourcesBuildPhase section */
		C1000002 /* Sources */ = {
			isa = PBXSourcesBuildPhase;
			buildActionMask = 2147483647;
			files = (
				A1000001 /* AppDelegate.swift in Sources */,
				A1000002 /* SceneDelegate.swift in Sources */,
				A1000003 /* ViewController.swift in Sources */,
			);
			runOnlyForDeploymentPostprocessing = 0;
		};
/* End PBXSourcesBuildPhase section */

/* Begin XCBuildConfiguration section */
		G1000001 /* Debug */ = {
			isa = XCBuildConfiguration;
			buildSettings = {
				ALWAYS_SEARCH_USER_PATHS = NO;
				CLANG_ANALYZER_NONNULL = YES;
				CLANG_CXX_LANGUAGE_STANDARD = "gnu++20";
				CLANG_ENABLE_MODULES = YES;
				CLANG_ENABLE_OBJC_ARC = YES;
				COPY_PHASE_STRIP = NO;
				DEBUG_INFORMATION_FORMAT = dwarf;
				ENABLE_STRICT_OBJC_MSGSEND = YES;
				ENABLE_TESTABILITY = YES;
				GCC_DYNAMIC_NO_PIC = NO;
				GCC_NO_COMMON_BLOCKS = YES;
				GCC_OPTIMIZATION_LEVEL = 0;
				GCC_PREPROCESSOR_DEFINITIONS = (
					"DEBUG=1",
					"$(inherited)",
				);
				IPHONEOS_DEPLOYMENT_TARGET = 15.0;
				MTL_ENABLE_DEBUG_INFO = INCLUDE_SOURCE;
				ONLY_ACTIVE_ARCH = YES;
				SDKROOT = iphoneos;
				SWIFT_ACTIVE_COMPILATION_CONDITIONS = DEBUG;
				SWIFT_OPTIMIZATION_LEVEL = "-Onone";
			};
			name = Debug;
		};
		G1000002 /* Release */ = {
			isa = XCBuildConfiguration;
			buildSettings = {
				ALWAYS_SEARCH_USER_PATHS = NO;
				CLANG_ANALYZER_NONNULL = YES;
				CLANG_CXX_LANGUAGE_STANDARD = "gnu++20";
				CLANG_ENABLE_MODULES = YES;
				CLANG_ENABLE_OBJC_ARC = YES;
				COPY_PHASE_STRIP = NO;
				DEBUG_INFORMATION_FORMAT = "dwarf-with-dsym";
				ENABLE_NS_ASSERTIONS = NO;
				ENABLE_STRICT_OBJC_MSGSEND = YES;
				GCC_NO_COMMON_BLOCKS = YES;
				IPHONEOS_DEPLOYMENT_TARGET = 15.0;
				MTL_ENABLE_DEBUG_INFO = NO;
				SDKROOT = iphoneos;
				SWIFT_COMPILATION_MODE = wholemodule;
				SWIFT_OPTIMIZATION_LEVEL = "-O";
				VALIDATE_PRODUCT = YES;
			};
			name = Release;
		};
		G1000003 /* Debug */ = {
			isa = XCBuildConfiguration;
			buildSettings = {
				ASSETCATALOG_COMPILER_APPICON_NAME = AppIcon;
				CODE_SIGN_STYLE = Automatic;
				CURRENT_PROJECT_VERSION = 1;
				GENERATE_INFOPLIST_FILE = NO;
				INFOPLIST_FILE = ESP32LED/Info.plist;
				INFOPLIST_KEY_CFBundleDisplayName = "ESP32 LED";
				LD_RUNPATH_SEARCH_PATHS = (
					"$(inherited)",
					"@executable_path/Frameworks",
				);
				MARKETING_VERSION = 1.0;
				PRODUCT_BUNDLE_IDENTIFIER = com.hieu.esp32led;
				PRODUCT_NAME = "$(TARGET_NAME)";
				SWIFT_EMIT_LOC_STRINGS = YES;
				SWIFT_VERSION = 5.0;
				TARGETED_DEVICE_FAMILY = "1,2";
			};
			name = Debug;
		};
		G1000004 /* Release */ = {
			isa = XCBuildConfiguration;
			buildSettings = {
				ASSETCATALOG_COMPILER_APPICON_NAME = AppIcon;
				CODE_SIGN_STYLE = Automatic;
				CURRENT_PROJECT_VERSION = 1;
				GENERATE_INFOPLIST_FILE = NO;
				INFOPLIST_FILE = ESP32LED/Info.plist;
				INFOPLIST_KEY_CFBundleDisplayName = "ESP32 LED";
				LD_RUNPATH_SEARCH_PATHS = (
					"$(inherited)",
					"@executable_path/Frameworks",
				);
				MARKETING_VERSION = 1.0;
				PRODUCT_BUNDLE_IDENTIFIER = com.hieu.esp32led;
				PRODUCT_NAME = "$(TARGET_NAME)";
				SWIFT_EMIT_LOC_STRINGS = YES;
				SWIFT_VERSION = 5.0;
				TARGETED_DEVICE_FAMILY = "1,2";
			};
			name = Release;
		};
/* End XCBuildConfiguration section */

/* Begin XCConfigurationList section */
		F1000001 /* Build configuration list for PBXNativeTarget "ESP32LED" */ = {
			isa = XCConfigurationList;
			buildConfigurations = (
				G1000003 /* Debug */,
				G1000004 /* Release */,
			);
			defaultConfigurationIsVisible = 0;
			defaultConfigurationName = Release;
		};
		F1000002 /* Build configuration list for PBXProject "ESP32LED" */ = {
			isa = XCConfigurationList;
			buildConfigurations = (
				G1000001 /* Debug */,
				G1000002 /* Release */,
			);
			defaultConfigurationIsVisible = 0;
			defaultConfigurationName = Release;
		};
/* End XCConfigurationList section */
	};
	rootObject = F1000000 /* Project object */;
}
"""
with open(os.path.join(proj_dir, "project.pbxproj"), "w", encoding="utf-8") as f:
    f.write(pbxproj)

# 6. GitHub Actions Workflow
github_workflow = """name: Build iOS IPA

on:
  push:
    branches: [ "main", "master" ]
  workflow_dispatch:

jobs:
  build-ipa:
    runs-on: macos-14

    steps:
      - name: Checkout repository
        uses: actions/checkout@v4

      - name: Select Xcode 15
        run: sudo xcode-select -s /Applications/Xcode_15.4.app/Contents/Developer

      - name: Build Archive (Unsigned)
        run: |
          xcodebuild archive \\
            -project ios_app/ESP32LED.xcodeproj \\
            -scheme ESP32LED \\
            -configuration Release \\
            -archivePath build/ESP32LED.xcarchive \\
            CODE_SIGN_IDENTITY="" \\
            CODE_SIGNING_REQUIRED=NO \\
            CODE_SIGNING_ALLOWED=NO

      - name: Package IPA
        run: |
          mkdir -p build/Payload
          cp -R build/ESP32LED.xcarchive/Products/Applications/ESP32LED.app build/Payload/
          cd build
          zip -r ESP32LED.ipa Payload
          cd ..

      - name: Upload IPA Artifact
        uses: actions/upload-artifact@v4
        with:
          name: ESP32LED-iOS-IPA
          path: build/ESP32LED.ipa
"""
with open(os.path.join(github_dir, "build-ipa.yml"), "w", encoding="utf-8") as f:
    f.write(github_workflow)

print("iOS Xcode project and GitHub Actions workflow generated successfully!")
