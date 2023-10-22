#include <sal.h>

#include "Types.h"
constexpr f32 M_PI_4 = 0.7853981f;

#include <Windows.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <DirectXTK/Mouse.h>
#include <DirectXTK/WICTextureLoader.h>
#include <DirectXTK/DDSTextureLoader.h>
#include <DirectXTK/GeometricPrimitive.h>

using namespace DirectX;
using namespace Microsoft::WRL;

#include <string>
#include <map>
#include <iostream>
#include <algorithm>
#include <optional>
#include <initializer_list>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <cmath>
#include <cstdint>
#include <exception>
#include <profileapi.h>

using namespace std;

#include "FileHelpers.h"
#include "ErrorHelpers.h"

#pragma warning(push)
#pragma warning(disable: 4251)
#pragma warning(disable: 4275)
// see: https://github.com/Microsoft/vcpkg/issues/1621#issuecomment-321541039
#include "yaml-cpp/yaml.h"
#pragma warning(pop)

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

#include <GLTFSDK/GLTF.h>
#include <GLTFSDK/GLTFResourceReader.h>
#include <GLTFSDK/GLBResourceReader.h>
#include <GLTFSDK/Document.h>
#include <GLTFSDK/Deserialize.h>

//#include <PhysX/PxPhysicsAPI.h>
//#include <PhysX/cooking/PxCooking.h>
//using namespace physx;

#include "ManagerHelper.h"

#include "StepTimer.h"