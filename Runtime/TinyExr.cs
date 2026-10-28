using System;
using System.IO;
using System.Runtime.InteropServices;
using UnityEngine;

public static class TinyExr
{
#if UNITY_IOS && !UNITY_EDITOR
    private const string NativeLibrary = "__Internal";
#else
    private const string NativeLibrary = "TinyExr";
#endif
    private const int RgbaChannelCount = 4;

    [DllImport(NativeLibrary, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr tinyexr_load_rgba_from_memory(
        [In] byte[] buffer,
        int len,
        out int width,
        out int height,
        out IntPtr errorMessage);

    [DllImport(NativeLibrary, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr tinyexr_load_rgb9e5_from_memory(
        [In] byte[] buffer,
        int len,
        out int width,
        out int height,
        int flipVertically,
        out IntPtr errorMessage);

    [DllImport(NativeLibrary, CallingConvention = CallingConvention.Cdecl)]
    private static extern int tinyexr_is_exr_from_memory([In] byte[] buffer, int len);

    [DllImport(NativeLibrary, CallingConvention = CallingConvention.Cdecl)]
    private static extern void tinyexr_free(IntPtr ptr);

    [DllImport(NativeLibrary, CallingConvention = CallingConvention.Cdecl)]
    private static extern void tinyexr_free_error(IntPtr message);

    public sealed class ImageResult
    {
        public int width;
        public int height;
        public Color[] colors;
        public bool isHDR;
        public int sourceChannels;

        public Texture2D ToTexture2D(bool linear = true, bool mipChain = false)
        {
            var texture = new Texture2D(width, height, TextureFormat.RGBAFloat, mipChain, linear);
            texture.SetPixels(colors);
            texture.Apply(mipChain, false);
            return texture;
        }
    }

    public static bool IsEXR(byte[] bytes)
    {
        return bytes != null &&
               bytes.Length > 0 &&
               tinyexr_is_exr_from_memory(bytes, bytes.Length) != 0;
    }

    public static ImageResult LoadFile(string path)
    {
        return LoadFile(path, true);
    }

    public static ImageResult LoadFile(string path, bool flipVertically)
    {
        if (string.IsNullOrEmpty(path))
            throw new ArgumentException("EXR path is empty.", nameof(path));

        return Load(File.ReadAllBytes(path), flipVertically);
    }

    public static ImageResult Load(byte[] bytes)
    {
        return Load(bytes, true);
    }

    public static ImageResult Load(byte[] bytes, bool flipVertically)
    {
        if (bytes == null)
            throw new ArgumentNullException(nameof(bytes));
        if (bytes.Length == 0)
            throw new ArgumentException("EXR data is empty.", nameof(bytes));

        IntPtr errorMessage;
        var data = tinyexr_load_rgba_from_memory(
            bytes,
            bytes.Length,
            out int width,
            out int height,
            out errorMessage);

        if (data == IntPtr.Zero)
            throw CreateLoadException(errorMessage);

        try
        {
            if (width <= 0 || height <= 0)
                throw new InvalidOperationException("TinyEXR returned an invalid image size.");

            int pixelCount = checked(width * height);
            int floatCount = checked(pixelCount * RgbaChannelCount);

            var rgba = new float[floatCount];
            Marshal.Copy(data, rgba, 0, floatCount);

            var colors = new Color[pixelCount];
            CopyColors(rgba, width, height, flipVertically, colors);

            return new ImageResult()
            {
                width = width,
                height = height,
                colors = colors,
                isHDR = true,
                sourceChannels = RgbaChannelCount,
            };
        }
        finally
        {
            tinyexr_free(data);

            if (errorMessage != IntPtr.Zero)
                tinyexr_free_error(errorMessage);
        }
    }

    public static Color[] LoadColors(byte[] bytes, out int width, out int height)
    {
        var image = Load(bytes);
        width = image.width;
        height = image.height;
        return image.colors;
    }

    public static Texture2D LoadTexture2D(byte[] bytes, bool linear = true, bool mipChain = false)
    {
        return Load(bytes).ToTexture2D(linear, mipChain);
    }

    public static Texture2D LoadRgb9e5Texture2D(
        byte[] bytes,
        bool flipVertically = true,
        bool makeNoLongerReadable = true)
    {
        if (bytes == null)
            throw new ArgumentNullException(nameof(bytes));
        if (bytes.Length == 0)
            throw new ArgumentException("EXR data is empty.", nameof(bytes));

        IntPtr errorMessage;
        IntPtr data = tinyexr_load_rgb9e5_from_memory(
            bytes,
            bytes.Length,
            out int width,
            out int height,
            flipVertically ? 1 : 0,
            out errorMessage);

        if (data == IntPtr.Zero)
            throw CreateLoadException(errorMessage);

        Texture2D texture = null;
        try
        {
            if (width <= 0 || height <= 0)
                throw new InvalidOperationException("TinyEXR returned an invalid image size.");

            int byteCount = checked(width * height * sizeof(uint));
            texture = new Texture2D(
                width, height, TextureFormat.RGB9e5Float, false, true);
            texture.LoadRawTextureData(data, byteCount);
            texture.Apply(false, makeNoLongerReadable);
            return texture;
        }
        catch
        {
            if (texture != null)
                UnityEngine.Object.Destroy(texture);
            throw;
        }
        finally
        {
            tinyexr_free(data);

            if (errorMessage != IntPtr.Zero)
                tinyexr_free_error(errorMessage);
        }
    }

    private static void CopyColors(
        float[] rgba,
        int width,
        int height,
        bool flipVertically,
        Color[] colors)
    {
        for (int y = 0; y < height; y++)
        {
            int sourceY = flipVertically ? height - 1 - y : y;
            int sourceRow = sourceY * width;
            int destinationRow = y * width;

            for (int x = 0; x < width; x++)
            {
                int sourcePixel = (sourceRow + x) * RgbaChannelCount;
                colors[destinationRow + x] = new Color(
                    rgba[sourcePixel + 0],
                    rgba[sourcePixel + 1],
                    rgba[sourcePixel + 2],
                    rgba[sourcePixel + 3]);
            }
        }
    }

    private static InvalidOperationException CreateLoadException(IntPtr errorMessage)
    {
        try
        {
            string reason = errorMessage == IntPtr.Zero
                ? "Unknown reason."
                : Marshal.PtrToStringAnsi(errorMessage);

            return new InvalidOperationException($"TinyEXR failed to read EXR data: {reason}");
        }
        finally
        {
            if (errorMessage != IntPtr.Zero)
                tinyexr_free_error(errorMessage);
        }
    }
}
