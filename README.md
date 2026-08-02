# Unity TinyEXR

[syoyo/tinyexr](https://github.com/syoyo/tinyexr) wrapper for Unity.

It loads EXR bytes or files at runtime and returns `UnityEngine.Color[]`.

## Supported platforms

- Windows x86-64
- Linux x86-64
- macOS x86-64 and Apple silicon (universal binary)
- Android ARM64
- iOS ARM64

Native plugins for every platform are built by GitHub Actions. The workflow also
assembles a complete UPM tarball containing all five platform artifacts.

## Install with Unity Package Manager

Install a package tarball produced by the `Build native plugins` workflow, or
add a released package URL through Unity Package Manager.

The package identifier is `com.icosa.unitytinyexr`.

## Build the native plugin

```powershell
cmake -S "Native~" -B build/windows -A x64
cmake --build build/windows --config Release
```

By default, the plugin is emitted to `build/windows/plugin`. Override the
destination with `TINYEXR_UNITY_OUTPUT_DIRECTORY`.

For example:

```powershell
cmake -S "Native~" -B build/windows -A x64 `
  -DTINYEXR_UNITY_OUTPUT_DIRECTORY="$PWD/Runtime/Plugins/Windows/x86_64"
```

## Project layout

```text
Runtime/                         Unity runtime assembly and native plugins
Native~/                         Native source and CMake project
.github/workflows/               Cross-platform builds and UPM packaging
```

## Unity usage

```csharp
using UnityEngine;

public class ExrExample : MonoBehaviour
{
    public string path;

    void Start()
    {
        TinyExr.ImageResult image = TinyExr.LoadFile(path);

        int width = image.width;
        int height = image.height;
        Color[] colors = image.colors;

        Texture2D texture = image.ToTexture2D(linear: true);
    }
}
```

For only `Color[]`:

```csharp
Color[] colors = TinyExr.LoadColors(bytes, out int width, out int height);
```

For HDR backgrounds, use the packed texture path. It avoids managed float and
`Color[]` copies, uploads four bytes per pixel, and releases Unity's CPU texture
copy after upload:

```csharp
Texture2D texture = TinyExr.LoadRgb9e5Texture2D(bytes);
```

`Load(...)` and `LoadFile(...)` flip vertically by default to match Unity texture usage. Pass `false` to keep the raw row order produced by tinyexr.

## Notes

- Uses tinyexr's stable v1 `LoadEXRFromMemory` API and bundled `miniz`.
- Returns RGBA float data, mapped one-to-one into `UnityEngine.Color`.
- Layered EXR channels such as `diffuse.R` require tinyexr's layer-specific API and are not exposed by this minimal wrapper yet.
- iOS statically links the native library and resolves it through `__Internal`.
- Android currently targets ARM64, matching modern Unity mobile builds.
