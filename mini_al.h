// Audio playback and capture library. Public domain. See "unlicense" statement at the end of this file.
// mini_al - v0.8.13 - 2018-12-04
//
// David Reid - davidreidsoftware@gmail.com

// ABOUT
// =====
// mini_al is a small library for making it easy to connect to a playback or capture device and send
// or receive data from that device.
//
// mini_al uses an asynchronous API. Every device is created with it's own thread, with audio data
// being delivered to or from the device via a callback. Synchronous APIs are not supported in the
// interest of keeping the library as simple and light-weight as possible.
//
// Supported Backends:
//   - WASAPI
//   - DirectSound
//   - WinMM
//   - Core Audio (Apple)
//   - ALSA
//   - PulseAudio
//   - JACK
//   - sndio (OpenBSD)
//   - audio(4) (NetBSD and OpenBSD)
//   - OSS (FreeBSD)
//   - OpenSL|ES (Android only)
//   - OpenAL
//   - SDL2
//   - Null (Silence)
//
// Supported Formats:
//   - Unsigned 8-bit PCM
//   - Signed 16-bit PCM
//   - Signed 24-bit PCM (tightly packed)
//   - Signed 32-bit PCM
//   - IEEE 32-bit floating point PCM
//
//
// USAGE
// =====
// mini_al is a single-file library. To use it, do something like the following in one .c file.
//   #define MINI_AL_IMPLEMENTATION
//   #include "mini_al.h"
//
// You can then #include this file in other parts of the program as you would with any other header file.
//
// If you want to disable a specific backend, #define the appropriate MAL_NO_* option before the implementation.
//
// Note that GCC and Clang requires "-msse2", "-mavx2", etc. for SIMD optimizations.
//
//
// Building for Windows
// --------------------
// The Windows build should compile clean on all popular compilers without the need to configure any include paths
// nor link to any libraries.
//
// Building for macOS and iOS
// --------------------------
// The macOS build should compile clean without the need to download any dependencies or link to any libraries or
// frameworks. The iOS build needs to be compiled as Objective-C (sorry) and will need to link the relevant frameworks
// but should Just Work with Xcode.
//
// Building for Linux
// ------------------
// The Linux build only requires linking to -ldl, -lpthread and -lm. You do not need any development packages for any
// of the supported backends.
//
// Building for BSD
// ----------------
// The BSD build only requires linking to -ldl, -lpthread and -lm. NetBSD uses audio(4), OpenBSD uses sndio and
// FreeBSD uses OSS.
//
// Building for Android
// --------------------
// The Android build uses OpenSL|ES, and will require an appropriate API level that supports OpenSL|ES. mini_al has
// been tested against API levels 16 and 21.
//
// Building for Emscripten
// -----------------------
// The Emscripten build uses SDL2 and requires "-s USE_SDL=2" on the command line.
//
//
// Playback Example
// ----------------
//   mal_uint32 on_send_samples(mal_device* pDevice, mal_uint32 frameCount, void* pSamples)
//   {
//       // This callback is set at initialization time and will be called when a playback device needs more
//       // data. You need to write as many frames as you can to pSamples (but no more than frameCount) and
//       // then return the number of frames you wrote.
//       //
//       // The user data (pDevice->pUserData) is set by mal_device_init().
//       return (mal_uint32)mal_decoder_read((mal_decoder*)pDevice->pUserData, frameCount, pSamples);
//   }
//
//   ...
//
//   mal_device_config config = mal_device_config_init_playback(decoder.outputFormat, decoder.outputChannels, decoder.outputSampleRate, on_send_frames_to_device);
//
//   mal_device device;
//   mal_result result = mal_device_init(NULL, mal_device_type_playback, NULL, &config, &decoder /*pUserData*/, &device);
//   if (result != MAL_SUCCESS) {
//       return -1;
//   }
//
//   mal_device_start(&device);     // The device is sleeping by default so you'll need to start it manually.
//
//   ...
//
//   mal_device_uninit(&device);    // This will stop the device so no need to do that manually.
//
//
//
// NOTES
// =====
// - This library uses an asynchronous API for delivering and requesting audio data. Each device will have
//   it's own worker thread which is managed by the library.
// - If mal_device_init() is called with a device that's not aligned to the platform's natural alignment
//   boundary (4 bytes on 32-bit, 8 bytes on 64-bit), it will _not_ be thread-safe. The reason for this
//   is that it depends on members of mal_device being correctly aligned for atomic assignments.
// - Sample data is always native-endian and interleaved. For example, mal_format_s16 means signed 16-bit
//   integer samples, interleaved. Let me know if you need non-interleaved and I'll look into it.
// - The sndio backend is currently only enabled on OpenBSD builds.
// - The audio(4) backend is supported on OpenBSD, but you may need to disable sndiod before you can use it.
// - If you are using the platform's default device, mini_al will try automatically switching the internal
//   device when the device is unplugged. This feature is disabled when the device is opened in exclusive
//   mode.
//
//
//
// BACKEND NUANCES
// ===============
//
// PulseAudio
// ----------
// - If you experience bad glitching/noise on Arch Linux, consider this fix from the Arch wiki:
//     https://wiki.archlinux.org/index.php/PulseAudio/Troubleshooting#Glitches,_skips_or_crackling
//   Alternatively, consider using a different backend such as ALSA.
//
// Android
// -------
// - To capture audio on Android, remember to add the RECORD_AUDIO permission to your manifest:
//     <uses-permission android:name="android.permission.RECORD_AUDIO" />
// - Only a single mal_context can be active at any given time. This is due to a limitation with OpenSL|ES.
//
// UWP
// ---
// - UWP only supports default playback and capture devices.
// - UWP requires the Microphone capability to be enabled in the application's manifest (Package.appxmanifest):
//       <Package ...>
//           ...
//           <Capabilities>
//               <DeviceCapability Name="microphone" />
//           </Capabilities>
//       </Package>
//
// OpenAL
// ------
// - Capture is not supported on iOS with OpenAL. Use the Core Audio backend instead.
//
//
// OPTIONS
// =======
// #define these options before including this file.
//
// #define MAL_NO_WASAPI
//   Disables the WASAPI backend.
//
// #define MAL_NO_DSOUND
//   Disables the DirectSound backend.
//
// #define MAL_NO_WINMM
//   Disables the WinMM backend.
//
// #define MAL_NO_ALSA
//   Disables the ALSA backend.
//
// #define MAL_NO_PULSEAUDIO
//   Disables the PulseAudio backend.
//
// #define MAL_NO_JACK
//   Disables the JACK backend.
//
// #define MAL_NO_COREAUDIO
//   Disables the Core Audio backend.
//
// #define MAL_NO_SNDIO
//   Disables the sndio backend.
//
// #define MAL_NO_AUDIO4
//   Disables the audio(4) backend.
//
// #define MAL_NO_OSS
//   Disables the OSS backend.
//
// #define MAL_NO_OPENSL
//   Disables the OpenSL|ES backend.
//
// #define MAL_NO_OPENAL
//   Disables the OpenAL backend.
//
// #define MAL_NO_SDL
//   Disables the SDL backend.
//
// #define MAL_NO_NULL
//   Disables the null backend.
//
// #define MAL_DEFAULT_PERIODS
//   When a period count of 0 is specified when a device is initialized, it will default to this.
//
// #define MAL_BASE_BUFFER_SIZE_IN_MILLISECONDS_LOW_LATENCY
// #define MAL_BASE_BUFFER_SIZE_IN_MILLISECONDS_CONSERVATIVE
//   When a buffer size of 0 is specified when a device is initialized it will default to a buffer of this size, depending
//   on the chosen performance profile. These can be increased or decreased depending on your specific requirements.
//
// #define MAL_NO_DECODING
//   Disables the decoding APIs.
//
// #define MAL_NO_DEVICE_IO
//   Disables playback and recording. This will disable mal_context and mal_device APIs. This is useful if you only want to
//   use mini_al's data conversion and/or decoding APIs. 
//
// #define MAL_NO_STDIO
//   Disables file IO APIs.
//
// #define MAL_NO_SSE2
//   Disables SSE2 optimizations.
//
// #define MAL_NO_AVX2
//   Disables AVX2 optimizations.
//
// #define MAL_NO_AVX512
//   Disables AVX-512 optimizations.
//
// #define MAL_NO_NEON
//   Disables NEON optimizations.
//
// #define MAL_LOG_LEVEL <Level>
//   Sets the logging level. Set level to one of the following:
//     MAL_LOG_LEVEL_VERBOSE
//     MAL_LOG_LEVEL_INFO
//     MAL_LOG_LEVEL_WARNING
//     MAL_LOG_LEVEL_ERROR
//
// #define MAL_DEBUT_OUTPUT
//   Enable printf() debug output.
//
// #ifndef MAL_COINIT_VALUE
//   Windows only. The value to pass to internal calls to CoInitializeEx(). Defaults to COINIT_MULTITHREADED.

#ifndef mini_al_h
#define mini_al_h

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable:4201)   // nonstandard extension used: nameless struct/union
    #pragma warning(disable:4324)   // structure was padded due to alignment specifier
#endif

// Platform/backend detection.
#ifdef _WIN32
    #define MAL_WIN32
    #if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_PC_APP || WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)
        #define MAL_WIN32_UWP
    #else
        #define MAL_WIN32_DESKTOP
    #endif
#else
    #define MAL_POSIX
    #include <pthread.h>    // Unfortunate #include, but needed for pthread_t, pthread_mutex_t and pthread_cond_t types.

    #ifdef __unix__
        #define MAL_UNIX
        #if defined(__DragonFly__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
            #define MAL_BSD
        #endif
    #endif
    #ifdef __linux__
        #define MAL_LINUX
    #endif
    #ifdef __APPLE__
        #define MAL_APPLE
    #endif
    #ifdef __ANDROID__
        #define MAL_ANDROID
    #endif
    #ifdef __EMSCRIPTEN__
        #define MAL_EMSCRIPTEN
    #endif
#endif

#include <stddef.h> // For size_t.

#ifndef MAL_HAS_STDINT
    #if defined(_MSC_VER)
        #if _MSC_VER >= 1600
            #define MAL_HAS_STDINT
        #endif
    #else
        #if defined(__has_include)
            #if __has_include(<stdint.h>)
                #define MAL_HAS_STDINT
            #endif
        #endif
    #endif
#endif

#if !defined(MAL_HAS_STDINT) && (defined(__GNUC__) || defined(__clang__))   // Assume support for stdint.h on GCC and Clang.
    #define MAL_HAS_STDINT
#endif

#ifndef MAL_HAS_STDINT
typedef   signed char               mal_int8;
typedef unsigned char               mal_uint8;
typedef   signed short              mal_int16;
typedef unsigned short              mal_uint16;
typedef   signed int                mal_int32;
typedef unsigned int                mal_uint32;
    #if defined(_MSC_VER)
    typedef   signed __int64        mal_int64;
    typedef unsigned __int64        mal_uint64;
    #else
    typedef   signed long long int  mal_int64;
    typedef unsigned long long int  mal_uint64;
    #endif
    #if defined(_WIN32)
        #if defined(_WIN64)
        typedef mal_uint64          mal_uintptr;
        #else
        typedef mal_uint32          mal_uintptr;
        #endif
    #elif defined(__GNUC__)
        #if defined(__LP64__)
        typedef mal_uint64          mal_uintptr;
        #else
        typedef mal_uint32          mal_uintptr;
        #endif
    #else
        typedef mal_uint64          mal_uintptr;    // Fallback.
    #endif
#else
#include <stdint.h>
typedef int8_t                      mal_int8;
typedef uint8_t                     mal_uint8;
typedef int16_t                     mal_int16;
typedef uint16_t                    mal_uint16;
typedef int32_t                     mal_int32;
typedef uint32_t                    mal_uint32;
typedef int64_t                     mal_int64;
typedef uint64_t                    mal_uint64;
typedef uintptr_t                   mal_uintptr;
#endif
typedef mal_uint8                   mal_bool8;
typedef mal_uint32                  mal_bool32;
#define MAL_TRUE                    1
#define MAL_FALSE                   0

typedef void* mal_handle;
typedef void* mal_ptr;
typedef void (* mal_proc)(void);

#if defined(_MSC_VER) && !defined(_WCHAR_T_DEFINED)
typedef mal_uint16 wchar_t;
#endif

// Define NULL for some compilers.
#ifndef NULL
#define NULL 0
#endif

#if defined(SIZE_MAX)
    #define MAL_SIZE_MAX    SIZE_MAX
#else
    #define MAL_SIZE_MAX    0xFFFFFFFF  /* When SIZE_MAX is not defined by the standard library just default to the maximum 32-bit unsigned integer. */
#endif


#ifdef _MSC_VER
#define MAL_INLINE __forceinline
#else
#ifdef __GNUC__
#define MAL_INLINE inline __attribute__((always_inline))
#else
#define MAL_INLINE inline
#endif
#endif

#ifdef _MSC_VER
#define MAL_ALIGN(alignment) __declspec(align(alignment))
#elif !defined(__DMC__)
#define MAL_ALIGN(alignment) __attribute__((aligned(alignment)))
#else
#define MAL_ALIGN(alignment)
#endif

#ifdef _MSC_VER
#define MAL_ALIGNED_STRUCT(alignment) MAL_ALIGN(alignment) struct
#else
#define MAL_ALIGNED_STRUCT(alignment) struct MAL_ALIGN(alignment)
#endif

// SIMD alignment in bytes. Currently set to 64 bytes in preparation for future AVX-512 optimizations.
#define MAL_SIMD_ALIGNMENT  64


// Logging levels
#define MAL_LOG_LEVEL_VERBOSE   4
#define MAL_LOG_LEVEL_INFO      3
#define MAL_LOG_LEVEL_WARNING   2
#define MAL_LOG_LEVEL_ERROR     1

#ifndef MAL_LOG_LEVEL
#define MAL_LOG_LEVEL           MAL_LOG_LEVEL_ERROR
#endif

typedef struct mal_context mal_context;
typedef struct mal_device mal_device;

typedef mal_uint8 mal_channel;
#define MAL_CHANNEL_NONE                                0
#define MAL_CHANNEL_MONO                                1
#define MAL_CHANNEL_FRONT_LEFT                          2
#define MAL_CHANNEL_FRONT_RIGHT                         3
#define MAL_CHANNEL_FRONT_CENTER                        4
#define MAL_CHANNEL_LFE                                 5
#define MAL_CHANNEL_BACK_LEFT                           6
#define MAL_CHANNEL_BACK_RIGHT                          7
#define MAL_CHANNEL_FRONT_LEFT_CENTER                   8
#define MAL_CHANNEL_FRONT_RIGHT_CENTER                  9
#define MAL_CHANNEL_BACK_CENTER                         10
#define MAL_CHANNEL_SIDE_LEFT                           11
#define MAL_CHANNEL_SIDE_RIGHT                          12
#define MAL_CHANNEL_TOP_CENTER                          13
#define MAL_CHANNEL_TOP_FRONT_LEFT                      14
#define MAL_CHANNEL_TOP_FRONT_CENTER                    15
#define MAL_CHANNEL_TOP_FRONT_RIGHT                     16
#define MAL_CHANNEL_TOP_BACK_LEFT                       17
#define MAL_CHANNEL_TOP_BACK_CENTER                     18
#define MAL_CHANNEL_TOP_BACK_RIGHT                      19
#define MAL_CHANNEL_AUX_0                               20
#define MAL_CHANNEL_AUX_1                               21
#define MAL_CHANNEL_AUX_2                               22
#define MAL_CHANNEL_AUX_3                               23
#define MAL_CHANNEL_AUX_4                               24
#define MAL_CHANNEL_AUX_5                               25
#define MAL_CHANNEL_AUX_6                               26
#define MAL_CHANNEL_AUX_7                               27
#define MAL_CHANNEL_AUX_8                               28
#define MAL_CHANNEL_AUX_9                               29
#define MAL_CHANNEL_AUX_10                              30
#define MAL_CHANNEL_AUX_11                              31
#define MAL_CHANNEL_AUX_12                              32
#define MAL_CHANNEL_AUX_13                              33
#define MAL_CHANNEL_AUX_14                              34
#define MAL_CHANNEL_AUX_15                              35
#define MAL_CHANNEL_AUX_16                              36
#define MAL_CHANNEL_AUX_17                              37
#define MAL_CHANNEL_AUX_18                              38
#define MAL_CHANNEL_AUX_19                              39
#define MAL_CHANNEL_AUX_20                              40
#define MAL_CHANNEL_AUX_21                              41
#define MAL_CHANNEL_AUX_22                              42
#define MAL_CHANNEL_AUX_23                              43
#define MAL_CHANNEL_AUX_24                              44
#define MAL_CHANNEL_AUX_25                              45
#define MAL_CHANNEL_AUX_26                              46
#define MAL_CHANNEL_AUX_27                              47
#define MAL_CHANNEL_AUX_28                              48
#define MAL_CHANNEL_AUX_29                              49
#define MAL_CHANNEL_AUX_30                              50
#define MAL_CHANNEL_AUX_31                              51
#define MAL_CHANNEL_LEFT                                MAL_CHANNEL_FRONT_LEFT
#define MAL_CHANNEL_RIGHT                               MAL_CHANNEL_FRONT_RIGHT
#define MAL_CHANNEL_POSITION_COUNT                      MAL_CHANNEL_AUX_31 + 1

typedef int mal_result;
#define MAL_SUCCESS                                      0
#define MAL_ERROR                                       -1      // A generic error.
#define MAL_INVALID_ARGS                                -2
#define MAL_INVALID_OPERATION                           -3
#define MAL_OUT_OF_MEMORY                               -4
#define MAL_FORMAT_NOT_SUPPORTED                        -5
#define MAL_NO_BACKEND                                  -6
#define MAL_NO_DEVICE                                   -7
#define MAL_API_NOT_FOUND                               -8
#define MAL_DEVICE_BUSY                                 -9
#define MAL_DEVICE_NOT_INITIALIZED                      -10
#define MAL_DEVICE_NOT_STARTED                          -11
#define MAL_DEVICE_NOT_STOPPED                          -12
#define MAL_DEVICE_ALREADY_STARTED                      -13
#define MAL_DEVICE_ALREADY_STARTING                     -14
#define MAL_DEVICE_ALREADY_STOPPED                      -15
#define MAL_DEVICE_ALREADY_STOPPING                     -16
#define MAL_FAILED_TO_MAP_DEVICE_BUFFER                 -17
#define MAL_FAILED_TO_UNMAP_DEVICE_BUFFER               -18
#define MAL_FAILED_TO_INIT_BACKEND                      -19
#define MAL_FAILED_TO_READ_DATA_FROM_CLIENT             -20
#define MAL_FAILED_TO_READ_DATA_FROM_DEVICE             -21
#define MAL_FAILED_TO_SEND_DATA_TO_CLIENT               -22
#define MAL_FAILED_TO_SEND_DATA_TO_DEVICE               -23
#define MAL_FAILED_TO_OPEN_BACKEND_DEVICE               -24
#define MAL_FAILED_TO_START_BACKEND_DEVICE              -25
#define MAL_FAILED_TO_STOP_BACKEND_DEVICE               -26
#define MAL_FAILED_TO_CONFIGURE_BACKEND_DEVICE          -27
#define MAL_FAILED_TO_CREATE_MUTEX                      -28
#define MAL_FAILED_TO_CREATE_EVENT                      -29
#define MAL_FAILED_TO_CREATE_THREAD                     -30
#define MAL_INVALID_DEVICE_CONFIG                       -31
#define MAL_ACCESS_DENIED                               -32
#define MAL_TOO_LARGE                                   -33
#define MAL_DEVICE_UNAVAILABLE                          -34
#define MAL_TIMEOUT                                     -35

// Standard sample rates.
#define MAL_SAMPLE_RATE_8000                            8000
#define MAL_SAMPLE_RATE_11025                           11025
#define MAL_SAMPLE_RATE_16000                           16000
#define MAL_SAMPLE_RATE_22050                           22050
#define MAL_SAMPLE_RATE_24000                           24000
#define MAL_SAMPLE_RATE_32000                           32000
#define MAL_SAMPLE_RATE_44100                           44100
#define MAL_SAMPLE_RATE_48000                           48000
#define MAL_SAMPLE_RATE_88200                           88200
#define MAL_SAMPLE_RATE_96000                           96000
#define MAL_SAMPLE_RATE_176400                          176400
#define MAL_SAMPLE_RATE_192000                          192000
#define MAL_SAMPLE_RATE_352800                          352800
#define MAL_SAMPLE_RATE_384000                          384000

#define MAL_MIN_PCM_SAMPLE_SIZE_IN_BYTES                1   // For simplicity, mini_al does not support PCM samples that are not byte aligned.
#define MAL_MAX_PCM_SAMPLE_SIZE_IN_BYTES                8
#define MAL_MIN_CHANNELS                                1
#define MAL_MAX_CHANNELS                                32
#define MAL_MIN_SAMPLE_RATE                             MAL_SAMPLE_RATE_8000
#define MAL_MAX_SAMPLE_RATE                             MAL_SAMPLE_RATE_384000
#define MAL_SRC_SINC_MIN_WINDOW_WIDTH                   2
#define MAL_SRC_SINC_MAX_WINDOW_WIDTH                   32
#define MAL_SRC_SINC_DEFAULT_WINDOW_WIDTH               32
#define MAL_SRC_SINC_LOOKUP_TABLE_RESOLUTION            8
#define MAL_SRC_INPUT_BUFFER_SIZE_IN_SAMPLES            256

typedef enum
{
    mal_stream_format_pcm = 0,
} mal_stream_format;

typedef enum
{
    mal_stream_layout_interleaved = 0,
    mal_stream_layout_deinterleaved
} mal_stream_layout;

typedef enum
{
    mal_dither_mode_none = 0,
    mal_dither_mode_rectangle,
    mal_dither_mode_triangle
} mal_dither_mode;

typedef enum
{
    // I like to keep these explicitly defined because they're used as a key into a lookup table. When items are
    // added to this, make sure there are no gaps and that they're added to the lookup table in mal_get_bytes_per_sample().
    mal_format_unknown = 0,     // Mainly used for indicating an error, but also used as the default for the output format for decoders.
    mal_format_u8      = 1,
    mal_format_s16     = 2,     // Seems to be the most widely supported format.
    mal_format_s24     = 3,     // Tightly packed. 3 bytes per sample.
    mal_format_s32     = 4,
    mal_format_f32     = 5,
    mal_format_count
} mal_format;

typedef enum
{
    mal_channel_mix_mode_planar_blend = 0,  // Simple averaging based on the plane(s) the channel is sitting on.
    mal_channel_mix_mode_simple,            // Drop excess channels; zeroed out extra channels.
    mal_channel_mix_mode_default = mal_channel_mix_mode_planar_blend
} mal_channel_mix_mode;

typedef enum
{
    mal_standard_channel_map_microsoft,
    mal_standard_channel_map_alsa,
    mal_standard_channel_map_rfc3551,   // Based off AIFF.
    mal_standard_channel_map_flac,
    mal_standard_channel_map_vorbis,
    mal_standard_channel_map_sound4,    // FreeBSD's sound(4).
    mal_standard_channel_map_sndio,     // www.sndio.org/tips.html
    mal_standard_channel_map_default = mal_standard_channel_map_microsoft
} mal_standard_channel_map;

typedef enum
{
    mal_performance_profile_low_latency = 0,
    mal_performance_profile_conservative
} mal_performance_profile;


typedef struct mal_format_converter mal_format_converter;
typedef mal_uint32 (* mal_format_converter_read_proc)              (mal_format_converter* pConverter, mal_uint32 frameCount, void* pFramesOut, void* pUserData);
typedef mal_uint32 (* mal_format_converter_read_deinterleaved_proc)(mal_format_converter* pConverter, mal_uint32 frameCount, void** ppSamplesOut, void* pUserData);

typedef struct
{
    mal_format formatIn;
    mal_format formatOut;
    mal_uint32 channels;
    mal_stream_format streamFormatIn;
    mal_stream_format streamFormatOut;
    mal_dither_mode ditherMode;
    mal_bool32 noSSE2   : 1;
    mal_bool32 noAVX2   : 1;
    mal_bool32 noAVX512 : 1;
    mal_bool32 noNEON   : 1;
    mal_format_converter_read_proc onRead;
    mal_format_converter_read_deinterleaved_proc onReadDeinterleaved;
    void* pUserData;
} mal_format_converter_config;

struct mal_format_converter
{
    mal_format_converter_config config;
    mal_bool32 useSSE2   : 1;
    mal_bool32 useAVX2   : 1;
    mal_bool32 useAVX512 : 1;
    mal_bool32 useNEON   : 1;
    void (* onConvertPCM)(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode);
    void (* onInterleavePCM)(void* dst, const void** src, mal_uint64 frameCount, mal_uint32 channels);
    void (* onDeinterleavePCM)(void** dst, const void* src, mal_uint64 frameCount, mal_uint32 channels);
};



typedef struct mal_channel_router mal_channel_router;
typedef mal_uint32 (* mal_channel_router_read_deinterleaved_proc)(mal_channel_router* pRouter, mal_uint32 frameCount, void** ppSamplesOut, void* pUserData);

typedef struct
{
    mal_uint32 channelsIn;
    mal_uint32 channelsOut;
    mal_channel channelMapIn[MAL_MAX_CHANNELS];
    mal_channel channelMapOut[MAL_MAX_CHANNELS];
    mal_channel_mix_mode mixingMode;
    mal_bool32 noSSE2   : 1;
    mal_bool32 noAVX2   : 1;
    mal_bool32 noAVX512 : 1;
    mal_bool32 noNEON   : 1;
    mal_channel_router_read_deinterleaved_proc onReadDeinterleaved;
    void* pUserData;
} mal_channel_router_config;

struct mal_channel_router
{
    mal_channel_router_config config;
    mal_bool32 isPassthrough   : 1;
    mal_bool32 isSimpleShuffle : 1;
    mal_bool32 useSSE2         : 1;
    mal_bool32 useAVX2         : 1;
    mal_bool32 useAVX512       : 1;
    mal_bool32 useNEON         : 1;
    mal_uint8 shuffleTable[MAL_MAX_CHANNELS];
    float weights[MAL_MAX_CHANNELS][MAL_MAX_CHANNELS];
};



typedef struct mal_src mal_src;
//typedef mal_uint32 (* mal_src_read_proc)(mal_src* pSRC, mal_uint32 frameCount, void* pFramesOut, void* pUserData); // Returns the number of frames that were read.
typedef mal_uint32 (* mal_src_read_deinterleaved_proc)(mal_src* pSRC, mal_uint32 frameCount, void** ppSamplesOut, void* pUserData); // Returns the number of frames that were read.

typedef enum
{
    mal_src_algorithm_sinc = 0,
    mal_src_algorithm_linear,
    mal_src_algorithm_none,
    mal_src_algorithm_default = mal_src_algorithm_sinc
} mal_src_algorithm;

typedef enum
{
    mal_src_sinc_window_function_hann = 0,
    mal_src_sinc_window_function_rectangular,
    mal_src_sinc_window_function_default = mal_src_sinc_window_function_hann
} mal_src_sinc_window_function;

typedef struct
{
    mal_src_sinc_window_function windowFunction;
    mal_uint32 windowWidth;
} mal_src_config_sinc;

typedef struct
{
    mal_uint32 sampleRateIn;
    mal_uint32 sampleRateOut;
    mal_uint32 channels;
    mal_src_algorithm algorithm;
    mal_bool32 neverConsumeEndOfInput : 1;
    mal_bool32 noSSE2   : 1;
    mal_bool32 noAVX2   : 1;
    mal_bool32 noAVX512 : 1;
    mal_bool32 noNEON   : 1;
    mal_src_read_deinterleaved_proc onReadDeinterleaved;
    void* pUserData;
    union
    {
        mal_src_config_sinc sinc;
    };
} mal_src_config;

MAL_ALIGNED_STRUCT(MAL_SIMD_ALIGNMENT) mal_src
{
    union
    {
        struct
        {
            MAL_ALIGN(MAL_SIMD_ALIGNMENT) float input[MAL_MAX_CHANNELS][MAL_SRC_INPUT_BUFFER_SIZE_IN_SAMPLES];
            float timeIn;
            mal_uint32 leftoverFrames;
        } linear;

        struct
        {
            MAL_ALIGN(MAL_SIMD_ALIGNMENT) float input[MAL_MAX_CHANNELS][MAL_SRC_SINC_MAX_WINDOW_WIDTH*2 + MAL_SRC_INPUT_BUFFER_SIZE_IN_SAMPLES];
            float timeIn;
            mal_uint32 inputFrameCount;     // The number of frames sitting in the input buffer, not including the first half of the window.
            mal_uint32 windowPosInSamples;  // An offset of <input>.
            float table[MAL_SRC_SINC_MAX_WINDOW_WIDTH*1 * MAL_SRC_SINC_LOOKUP_TABLE_RESOLUTION]; // Precomputed lookup table. The +1 is used to avoid the need for an overflow check.
        } sinc;
    };

    mal_src_config config;
    mal_bool32 isEndOfInputLoaded : 1;
    mal_bool32 useSSE2   : 1;
    mal_bool32 useAVX2   : 1;
    mal_bool32 useAVX512 : 1;
    mal_bool32 useNEON   : 1;
};

typedef struct mal_dsp mal_dsp;
typedef mal_uint32 (* mal_dsp_read_proc)(mal_dsp* pDSP, mal_uint32 frameCount, void* pSamplesOut, void* pUserData);

typedef struct
{
    mal_format formatIn;
    mal_uint32 channelsIn;
    mal_uint32 sampleRateIn;
    mal_channel channelMapIn[MAL_MAX_CHANNELS];
    mal_format formatOut;
    mal_uint32 channelsOut;
    mal_uint32 sampleRateOut;
    mal_channel channelMapOut[MAL_MAX_CHANNELS];
    mal_channel_mix_mode channelMixMode;
    mal_dither_mode ditherMode;
    mal_src_algorithm srcAlgorithm;
    mal_bool32 allowDynamicSampleRate;
    mal_bool32 neverConsumeEndOfInput : 1;  // <-- For SRC.
    mal_bool32 noSSE2   : 1;
    mal_bool32 noAVX2   : 1;
    mal_bool32 noAVX512 : 1;
    mal_bool32 noNEON   : 1;
    mal_dsp_read_proc onRead;
    void* pUserData;
    union
    {
        mal_src_config_sinc sinc;
    };
} mal_dsp_config;

MAL_ALIGNED_STRUCT(MAL_SIMD_ALIGNMENT) mal_dsp
{
    mal_dsp_read_proc onRead;
    void* pUserData;
    mal_format_converter formatConverterIn;             // For converting data to f32 in preparation for further processing.
    mal_format_converter formatConverterOut;            // For converting data to the requested output format. Used as the final step in the processing pipeline.
    mal_channel_router channelRouter;                   // For channel conversion.
    mal_src src;                                        // For sample rate conversion.
    mal_bool32 isDynamicSampleRateAllowed     : 1;      // mal_dsp_set_input_sample_rate() and mal_dsp_set_output_sample_rate() will fail if this is set to false.
    mal_bool32 isPreFormatConversionRequired  : 1;
    mal_bool32 isPostFormatConversionRequired : 1;
    mal_bool32 isChannelRoutingRequired       : 1;
    mal_bool32 isSRCRequired                  : 1;
    mal_bool32 isChannelRoutingAtStart        : 1;
    mal_bool32 isPassthrough                  : 1;      // <-- Will be set to true when the DSP pipeline is an optimized passthrough.
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// DATA CONVERSION
// ===============
//
// This section contains the APIs for data conversion. You will find everything here for channel mapping, sample format conversion, resampling, etc.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Channel Maps
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Helper for retrieving a standard channel map.
void mal_get_standard_channel_map(mal_standard_channel_map standardChannelMap, mal_uint32 channels, mal_channel channelMap[MAL_MAX_CHANNELS]);

// Copies a channel map.
void mal_channel_map_copy(mal_channel* pOut, const mal_channel* pIn, mal_uint32 channels);


// Determines whether or not a channel map is valid.
//
// A blank channel map is valid (all channels set to MAL_CHANNEL_NONE). The way a blank channel map is handled is context specific, but
// is usually treated as a passthrough.
//
// Invalid channel maps:
//   - A channel map with no channels
//   - A channel map with more than one channel and a mono channel
mal_bool32 mal_channel_map_valid(mal_uint32 channels, const mal_channel channelMap[MAL_MAX_CHANNELS]);

// Helper for comparing two channel maps for equality.
//
// This assumes the channel count is the same between the two.
mal_bool32 mal_channel_map_equal(mal_uint32 channels, const mal_channel channelMapA[MAL_MAX_CHANNELS], const mal_channel channelMapB[MAL_MAX_CHANNELS]);

// Helper for determining if a channel map is blank (all channels set to MAL_CHANNEL_NONE).
mal_bool32 mal_channel_map_blank(mal_uint32 channels, const mal_channel channelMap[MAL_MAX_CHANNELS]);

// Helper for determining whether or not a channel is present in the given channel map.
mal_bool32 mal_channel_map_contains_channel_position(mal_uint32 channels, const mal_channel channelMap[MAL_MAX_CHANNELS], mal_channel channelPosition);


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Format Conversion
// =================
// The format converter serves two purposes:
//   1) Conversion between data formats (u8 to f32, etc.)
//   2) Interleaving and deinterleaving
//
// When initializing a converter, you specify the input and output formats (u8, s16, etc.) and read callbacks. There are two read callbacks - one for
// interleaved input data (onRead) and another for deinterleaved input data (onReadDeinterleaved). You implement whichever is most convenient for you. You
// can implement both, but it's not recommended as it just introduces unnecessary complexity.
//
// To read data as interleaved samples, use mal_format_converter_read(). Otherwise use mal_format_converter_read_deinterleaved().
//
// Dithering
// ---------
// The format converter also supports dithering. Dithering can be set using ditherMode variable in the config, like so.
//
//   pConfig->ditherMode = mal_dither_mode_rectangle;
//
// The different dithering modes include the following, in order of efficiency:
//   - None:      mal_dither_mode_none
//   - Rectangle: mal_dither_mode_rectangle
//   - Triangle:  mal_dither_mode_triangle
//
// Note that even if the dither mode is set to something other than mal_dither_mode_none, it will be ignored for conversions where dithering is not needed.
// Dithering is available for the following conversions:
//   - s16 -> u8
//   - s24 -> u8
//   - s32 -> u8
//   - f32 -> u8
//   - s24 -> s16
//   - s32 -> s16
//   - f32 -> s16
//
// Note that it is not an error to pass something other than mal_dither_mode_none for conversions where dither is not used. It will just be ignored.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Initializes a format converter.
mal_result mal_format_converter_init(const mal_format_converter_config* pConfig, mal_format_converter* pConverter);

// Reads data from the format converter as interleaved channels.
mal_uint64 mal_format_converter_read(mal_format_converter* pConverter, mal_uint64 frameCount, void* pFramesOut, void* pUserData);

// Reads data from the format converter as deinterleaved channels.
mal_uint64 mal_format_converter_read_deinterleaved(mal_format_converter* pConverter, mal_uint64 frameCount, void** ppSamplesOut, void* pUserData);


// Helper for initializing a format converter config.
mal_format_converter_config mal_format_converter_config_init_new(void);
mal_format_converter_config mal_format_converter_config_init(mal_format formatIn, mal_format formatOut, mal_uint32 channels, mal_format_converter_read_proc onRead, void* pUserData);
mal_format_converter_config mal_format_converter_config_init_deinterleaved(mal_format formatIn, mal_format formatOut, mal_uint32 channels, mal_format_converter_read_deinterleaved_proc onReadDeinterleaved, void* pUserData);



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Channel Routing
// ===============
// There are two main things you can do with the channel router:
//   1) Rearrange channels
//   2) Convert from one channel count to another
//
// Channel Rearrangement
// ---------------------
// A simple example of channel rearrangement may be swapping the left and right channels in a stereo stream. To do this you just pass in the same channel
// count for both the input and output with channel maps that contain the same channels (in a different order).
//
// Channel Conversion
// ------------------
// The channel router can also convert from one channel count to another, such as converting a 5.1 stream to stero. When changing the channel count, the
// router will first perform a 1:1 mapping of channel positions that are present in both the input and output channel maps. The second thing it will do
// is distribute the input mono channel (if any) across all output channels, excluding any None and LFE channels. If there is an output mono channel, all
// input channels will be averaged, excluding any None and LFE channels.
//
// The last case to consider is when a channel position in the input channel map is not present in the output channel map, and vice versa. In this case the
// channel router will perform a blend of other related channels to produce an audible channel. There are several blending modes.
//   1) Simple
//      Unmatched channels are silenced.
//   2) Planar Blending
//      Channels are blended based on a set of planes that each speaker emits audio from.
//
// Planar Blending
// ---------------
// In this mode, channel positions are associated with a set of planes where the channel conceptually emits audio from. An example is the front/left speaker.
// This speaker is positioned to the front of the listener, so you can think of it as emitting audio from the front plane. It is also positioned to the left
// of the listener so you can think of it as also emitting audio from the left plane. Now consider the (unrealistic) situation where the input channel map
// contains only the front/left channel position, but the output channel map contains both the front/left and front/center channel. When deciding on the audio
// data to send to the front/center speaker (which has no 1:1 mapping with an input channel) we need to use some logic based on our available input channel
// positions.
//
// As mentioned earlier, our front/left speaker is, conceptually speaking, emitting audio from the front _and_ the left planes. Similarly, the front/center
// speaker is emitting audio from _only_ the front plane. What these two channels have in common is that they are both emitting audio from the front plane.
// Thus, it makes sense that the front/center speaker should receive some contribution from the front/left channel. How much contribution depends on their
// planar relationship (thus the name of this blending technique).
//
// Because the front/left channel is emitting audio from two planes (front and left), you can think of it as though it's willing to dedicate 50% of it's total
// volume to each of it's planes (a channel position emitting from 1 plane would be willing to given 100% of it's total volume to that plane, and a channel
// position emitting from 3 planes would be willing to given 33% of it's total volume to each plane). Similarly, the front/center speaker is emitting audio
// from only one plane so you can think of it as though it's willing to _take_ 100% of it's volume from front plane emissions. Now, since the front/left
// channel is willing to _give_ 50% of it's total volume to the front plane, and the front/center speaker is willing to _take_ 100% of it's total volume
// from the front, you can imagine that 50% of the front/left speaker will be given to the front/center speaker.
//
// Usage
// -----
// To use the channel router you need to specify three things:
//   1) The input channel count and channel map
//   2) The output channel count and channel map
//   3) The mixing mode to use in the case where a 1:1 mapping is unavailable
//
// Note that input and output data is always deinterleaved 32-bit floating point.
//
// Initialize the channel router with mal_channel_router_init(). You will need to pass in a config object which specifies the input and output configuration,
// mixing mode and a callback for sending data to the router. This callback will be called when input data needs to be sent to the router for processing.
//
// Read data from the channel router with mal_channel_router_read_deinterleaved(). Output data is always 32-bit floating point.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Initializes a channel router where it is assumed that the input data is non-interleaved.
mal_result mal_channel_router_init(const mal_channel_router_config* pConfig, mal_channel_router* pRouter);

// Reads data from the channel router as deinterleaved channels.
mal_uint64 mal_channel_router_read_deinterleaved(mal_channel_router* pRouter, mal_uint64 frameCount, void** ppSamplesOut, void* pUserData);

// Helper for initializing a channel router config.
mal_channel_router_config mal_channel_router_config_init(mal_uint32 channelsIn, const mal_channel channelMapIn[MAL_MAX_CHANNELS], mal_uint32 channelsOut, const mal_channel channelMapOut[MAL_MAX_CHANNELS], mal_channel_mix_mode mixingMode, mal_channel_router_read_deinterleaved_proc onRead, void* pUserData);


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Sample Rate Conversion
// ======================
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Initializes a sample rate conversion object.
mal_result mal_src_init(const mal_src_config* pConfig, mal_src* pSRC);

// Dynamically adjusts the input sample rate.
//
// DEPRECATED. Use mal_src_set_sample_rate() instead.
mal_result mal_src_set_input_sample_rate(mal_src* pSRC, mal_uint32 sampleRateIn);

// Dynamically adjusts the output sample rate.
//
// This is useful for dynamically adjust pitch. Keep in mind, however, that this will speed up or slow down the sound. If this
// is not acceptable you will need to use your own algorithm.
//
// DEPRECATED. Use mal_src_set_sample_rate() instead.
mal_result mal_src_set_output_sample_rate(mal_src* pSRC, mal_uint32 sampleRateOut);

// Dynamically adjusts the sample rate.
//
// This is useful for dynamically adjust pitch. Keep in mind, however, that this will speed up or slow down the sound. If this
// is not acceptable you will need to use your own algorithm.
mal_result mal_src_set_sample_rate(mal_src* pSRC, mal_uint32 sampleRateIn, mal_uint32 sampleRateOut);

// Reads a number of frames.
//
// Returns the number of frames actually read.
mal_uint64 mal_src_read_deinterleaved(mal_src* pSRC, mal_uint64 frameCount, void** ppSamplesOut, void* pUserData);


// Helper for creating a sample rate conversion config.
mal_src_config mal_src_config_init_new(void);
mal_src_config mal_src_config_init(mal_uint32 sampleRateIn, mal_uint32 sampleRateOut, mal_uint32 channels, mal_src_read_deinterleaved_proc onReadDeinterleaved, void* pUserData);


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// DSP
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Initializes a DSP object.
mal_result mal_dsp_init(const mal_dsp_config* pConfig, mal_dsp* pDSP);

// Dynamically adjusts the input sample rate.
//
// This will fail is the DSP was not initialized with allowDynamicSampleRate.
//
// DEPRECATED. Use mal_dsp_set_sample_rate() instead.
mal_result mal_dsp_set_input_sample_rate(mal_dsp* pDSP, mal_uint32 sampleRateOut);

// Dynamically adjusts the output sample rate.
//
// This is useful for dynamically adjust pitch. Keep in mind, however, that this will speed up or slow down the sound. If this
// is not acceptable you will need to use your own algorithm.
//
// This will fail is the DSP was not initialized with allowDynamicSampleRate.
//
// DEPRECATED. Use mal_dsp_set_sample_rate() instead.
mal_result mal_dsp_set_output_sample_rate(mal_dsp* pDSP, mal_uint32 sampleRateOut);

// Dynamically adjusts the output sample rate.
//
// This is useful for dynamically adjust pitch. Keep in mind, however, that this will speed up or slow down the sound. If this
// is not acceptable you will need to use your own algorithm.
//
// This will fail is the DSP was not initialized with allowDynamicSampleRate.
mal_result mal_dsp_set_sample_rate(mal_dsp* pDSP, mal_uint32 sampleRateIn, mal_uint32 sampleRateOut);


// Reads a number of frames and runs them through the DSP processor.
mal_uint64 mal_dsp_read(mal_dsp* pDSP, mal_uint64 frameCount, void* pFramesOut, void* pUserData);

// Helper for initializing a mal_dsp_config object.
mal_dsp_config mal_dsp_config_init_new(void);
mal_dsp_config mal_dsp_config_init(mal_format formatIn, mal_uint32 channelsIn, mal_uint32 sampleRateIn, mal_format formatOut, mal_uint32 channelsOut, mal_uint32 sampleRateOut, mal_dsp_read_proc onRead, void* pUserData);
mal_dsp_config mal_dsp_config_init_ex(mal_format formatIn, mal_uint32 channelsIn, mal_uint32 sampleRateIn, mal_channel channelMapIn[MAL_MAX_CHANNELS], mal_format formatOut, mal_uint32 channelsOut, mal_uint32 sampleRateOut,  mal_channel channelMapOut[MAL_MAX_CHANNELS], mal_dsp_read_proc onRead, void* pUserData);


// High-level helper for doing a full format conversion in one go. Returns the number of output frames. Call this with pOut set to NULL to
// determine the required size of the output buffer.
//
// A return value of 0 indicates an error.
//
// This function is useful for one-off bulk conversions, but if you're streaming data you should use the DSP APIs instead.
mal_uint64 mal_convert_frames(void* pOut, mal_format formatOut, mal_uint32 channelsOut, mal_uint32 sampleRateOut, const void* pIn, mal_format formatIn, mal_uint32 channelsIn, mal_uint32 sampleRateIn, mal_uint64 frameCountIn);
mal_uint64 mal_convert_frames_ex(void* pOut, mal_format formatOut, mal_uint32 channelsOut, mal_uint32 sampleRateOut, mal_channel channelMapOut[MAL_MAX_CHANNELS], const void* pIn, mal_format formatIn, mal_uint32 channelsIn, mal_uint32 sampleRateIn, mal_channel channelMapIn[MAL_MAX_CHANNELS], mal_uint64 frameCountIn);



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Miscellaneous Helpers
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// malloc(). Calls MAL_MALLOC().
void* mal_malloc(size_t sz);

// realloc(). Calls MAL_REALLOC().
void* mal_realloc(void* p, size_t sz);

// free(). Calls MAL_FREE().
void mal_free(void* p);

// Performs an aligned malloc, with the assumption that the alignment is a power of 2.
void* mal_aligned_malloc(size_t sz, size_t alignment);

// Free's an aligned malloc'd buffer.
void mal_aligned_free(void* p);

// Retrieves a friendly name for a format.
const char* mal_get_format_name(mal_format format);

// Blends two frames in floating point format.
void mal_blend_f32(float* pOut, float* pInA, float* pInB, float factor, mal_uint32 channels);

// Retrieves the size of a sample in bytes for the given format.
//
// This API is efficient and is implemented using a lookup table.
//
// Thread Safety: SAFE
//   This is API is pure.
mal_uint32 mal_get_bytes_per_sample(mal_format format);
static MAL_INLINE mal_uint32 mal_get_bytes_per_frame(mal_format format, mal_uint32 channels) { return mal_get_bytes_per_sample(format) * channels; }


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Format Conversion
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void mal_pcm_u8_to_s16(void* pOut, const void* pIn, mal_uint64 count, mal_dither_mode ditherMode);
void mal_pcm_u8_to_s24(void* pOut, const void* pIn, mal_uint64 count, mal_dither_mode ditherMode);
void mal_pcm_u8_to_s32(void* pOut, const void* pIn, mal_uint64 count, mal_dither_mode ditherMode);
void mal_pcm_u8_to_f32(void* pOut, const void* pIn, mal_uint64 count, mal_dither_mode ditherMode);
void mal_pcm_s16_to_u8(void* pOut, const void* pIn, mal_uint64 count, mal_dither_mode ditherMode);
void mal_pcm_s16_to_s24(void* pOut, const void* pIn, mal_uint64 count, mal_dither_mode ditherMode);
void mal_pcm_s16_to_s32(void* pOut, const void* pIn, mal_uint64 count, mal_dither_mode ditherMode);
void mal_pcm_s16_to_f32(void* pOut, const void* pIn, mal_uint64 count, mal_dither_mode ditherMode);
void mal_pcm_s24_to_u8(void* pOut, const void* pIn, mal_uint64 count, mal_dither_mode ditherMode);
void mal_pcm_s24_to_s16(void* pOut, const void* pIn, mal_uint64 count, mal_dither_mode ditherMode);
void mal_pcm_s24_to_s32(void* pOut, const void* pIn, mal_uint64 count, mal_dither_mode ditherMode);
void mal_pcm_s24_to_f32(void* pOut, const void* pIn, mal_uint64 count, mal_dither_mode ditherMode);
void mal_pcm_s32_to_u8(void* pOut, const void* pIn, mal_uint64 count, mal_dither_mode ditherMode);
void mal_pcm_s32_to_s16(void* pOut, const void* pIn, mal_uint64 count, mal_dither_mode ditherMode);
void mal_pcm_s32_to_s24(void* pOut, const void* pIn, mal_uint64 count, mal_dither_mode ditherMode);
void mal_pcm_s32_to_f32(void* pOut, const void* pIn, mal_uint64 count, mal_dither_mode ditherMode);
void mal_pcm_f32_to_u8(void* pOut, const void* pIn, mal_uint64 count, mal_dither_mode ditherMode);
void mal_pcm_f32_to_s16(void* pOut, const void* pIn, mal_uint64 count, mal_dither_mode ditherMode);
void mal_pcm_f32_to_s24(void* pOut, const void* pIn, mal_uint64 count, mal_dither_mode ditherMode);
void mal_pcm_f32_to_s32(void* pOut, const void* pIn, mal_uint64 count, mal_dither_mode ditherMode);
void mal_pcm_convert(void* pOut, mal_format formatOut, const void* pIn, mal_format formatIn, mal_uint64 sampleCount, mal_dither_mode ditherMode);

// Deinterleaves an interleaved buffer.
void mal_deinterleave_pcm_frames(mal_format format, mal_uint32 channels, mal_uint32 frameCount, const void* pInterleavedPCMFrames, void** ppDeinterleavedPCMFrames);

// Interleaves a group of deinterleaved buffers.
void mal_interleave_pcm_frames(mal_format format, mal_uint32 channels, mal_uint32 frameCount, const void** ppDeinterleavedPCMFrames, void* pInterleavedPCMFrames);


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// DEVICE I/O
// ==========
//
// This section contains the APIs for device playback and capture. Here is where you'll find mal_device_init(), etc.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef MAL_NO_DEVICE_IO
// Some backends are only supported on certain platforms.
#if defined(MAL_WIN32)
    #define MAL_SUPPORT_WASAPI
    #if defined(MAL_WIN32_DESKTOP)  // DirectSound and WinMM backends are only supported on desktop's.
        #define MAL_SUPPORT_DSOUND
        #define MAL_SUPPORT_WINMM
        #define MAL_SUPPORT_JACK    // JACK is technically supported on Windows, but I don't know how many people use it in practice...
    #endif
#endif
#if defined(MAL_UNIX)
    #if defined(MAL_LINUX)
        #if !defined(MAL_ANDROID)   // ALSA is not supported on Android.
            #define MAL_SUPPORT_ALSA
        #endif
    #endif
    #if !defined(MAL_BSD) && !defined(MAL_ANDROID) && !defined(MAL_EMSCRIPTEN)
        #define MAL_SUPPORT_PULSEAUDIO
        #define MAL_SUPPORT_JACK
    #endif
    #if defined(MAL_ANDROID)
        #define MAL_SUPPORT_OPENSL
    #endif
    #if defined(__OpenBSD__)    // <-- Change this to "#if defined(MAL_BSD)" to enable sndio on all BSD flavors.
        #define MAL_SUPPORT_SNDIO   // sndio is only supported on OpenBSD for now. May be expanded later if there's demand.
    #endif
    #if defined(__NetBSD__) || defined(__OpenBSD__)
        #define MAL_SUPPORT_AUDIO4  // Only support audio(4) on platforms with known support.
    #endif
    #if defined(__FreeBSD__) || defined(__DragonFly__)
        #define MAL_SUPPORT_OSS     // Only support OSS on specific platforms with known support.
    #endif
#endif
#if defined(MAL_APPLE)
    #define MAL_SUPPORT_COREAUDIO
#endif

#define MAL_SUPPORT_SDL     // All platforms support SDL.

// Explicitly disable OpenAL and Null backends for Emscripten because they both use a background thread which is not properly supported right now.
#if !defined(MAL_EMSCRIPTEN)
#define MAL_SUPPORT_OPENAL
#define MAL_SUPPORT_NULL    // All platforms support the null backend.
#endif


#if !defined(MAL_NO_WASAPI) && defined(MAL_SUPPORT_WASAPI)
    #define MAL_ENABLE_WASAPI
#endif
#if !defined(MAL_NO_DSOUND) && defined(MAL_SUPPORT_DSOUND)
    #define MAL_ENABLE_DSOUND
#endif
#if !defined(MAL_NO_WINMM) && defined(MAL_SUPPORT_WINMM)
    #define MAL_ENABLE_WINMM
#endif
#if !defined(MAL_NO_ALSA) && defined(MAL_SUPPORT_ALSA)
    #define MAL_ENABLE_ALSA
#endif
#if !defined(MAL_NO_PULSEAUDIO) && defined(MAL_SUPPORT_PULSEAUDIO)
    #define MAL_ENABLE_PULSEAUDIO
#endif
#if !defined(MAL_NO_JACK) && defined(MAL_SUPPORT_JACK)
    #define MAL_ENABLE_JACK
#endif
#if !defined(MAL_NO_COREAUDIO) && defined(MAL_SUPPORT_COREAUDIO)
    #define MAL_ENABLE_COREAUDIO
#endif
#if !defined(MAL_NO_SNDIO) && defined(MAL_SUPPORT_SNDIO)
    #define MAL_ENABLE_SNDIO
#endif
#if !defined(MAL_NO_AUDIO4) && defined(MAL_SUPPORT_AUDIO4)
    #define MAL_ENABLE_AUDIO4
#endif
#if !defined(MAL_NO_OSS) && defined(MAL_SUPPORT_OSS)
    #define MAL_ENABLE_OSS
#endif
#if !defined(MAL_NO_OPENSL) && defined(MAL_SUPPORT_OPENSL)
    #define MAL_ENABLE_OPENSL
#endif
#if !defined(MAL_NO_OPENAL) && defined(MAL_SUPPORT_OPENAL)
    #define MAL_ENABLE_OPENAL
#endif
#if !defined(MAL_NO_SDL) && defined(MAL_SUPPORT_SDL)
    #define MAL_ENABLE_SDL
#endif
#if !defined(MAL_NO_NULL) && defined(MAL_SUPPORT_NULL)
    #define MAL_ENABLE_NULL
#endif

#ifdef MAL_SUPPORT_WASAPI
// We need a IMMNotificationClient object for WASAPI. 
typedef struct
{
    void* lpVtbl;
    mal_uint32 counter;
    mal_device* pDevice;
} mal_IMMNotificationClient;
#endif


typedef enum
{
    mal_backend_null,
    mal_backend_wasapi,
    mal_backend_dsound,
    mal_backend_winmm,
    mal_backend_alsa,
    mal_backend_pulseaudio,
    mal_backend_jack,
    mal_backend_coreaudio,
    mal_backend_sndio,
    mal_backend_audio4,
    mal_backend_oss,
    mal_backend_opensl,
    mal_backend_openal,
    mal_backend_sdl
} mal_backend;

// Thread priorties should be ordered such that the default priority of the worker thread is 0.
typedef enum
{
    mal_thread_priority_idle     = -5,
    mal_thread_priority_lowest   = -4,
    mal_thread_priority_low      = -3,
    mal_thread_priority_normal   = -2,
    mal_thread_priority_high     = -1,
    mal_thread_priority_highest  =  0,
    mal_thread_priority_realtime =  1,
    mal_thread_priority_default  =  0
} mal_thread_priority;

typedef struct
{
    mal_context* pContext;

    union
    {
#ifdef MAL_WIN32
        struct
        {
            /*HANDLE*/ mal_handle hThread;
        } win32;
#endif
#ifdef MAL_POSIX
        struct
        {
            pthread_t thread;
        } posix;
#endif

        int _unused;
    };
} mal_thread;

typedef struct
{
    mal_context* pContext;

    union
    {
#ifdef MAL_WIN32
        struct
        {
            /*HANDLE*/ mal_handle hMutex;
        } win32;
#endif
#ifdef MAL_POSIX
        struct
        {
            pthread_mutex_t mutex;
        } posix;
#endif

        int _unused;
    };
} mal_mutex;

typedef struct
{
    mal_context* pContext;

    union
    {
#ifdef MAL_WIN32
        struct
        {
            /*HANDLE*/ mal_handle hEvent;
        } win32;
#endif
#ifdef MAL_POSIX
        struct
        {
            pthread_mutex_t mutex;
            pthread_cond_t condition;
            mal_uint32 value;
        } posix;
#endif

        int _unused;
    };
} mal_event;


#define MAL_MAX_PERIODS_DSOUND                          4
#define MAL_MAX_PERIODS_OPENAL                          4

typedef void       (* mal_log_proc) (mal_context* pContext, mal_device* pDevice, const char* message);
typedef void       (* mal_recv_proc)(mal_device* pDevice, mal_uint32 frameCount, const void* pSamples);
typedef mal_uint32 (* mal_send_proc)(mal_device* pDevice, mal_uint32 frameCount, void* pSamples);
typedef void       (* mal_stop_proc)(mal_device* pDevice);

typedef enum
{
    mal_device_type_playback,
    mal_device_type_capture
} mal_device_type;

typedef enum
{
    mal_share_mode_shared = 0,
    mal_share_mode_exclusive,
} mal_share_mode;

typedef union
{
#ifdef MAL_SUPPORT_WASAPI
    wchar_t wasapi[64];             // WASAPI uses a wchar_t string for identification.
#endif
#ifdef MAL_SUPPORT_DSOUND
    mal_uint8 dsound[16];           // DirectSound uses a GUID for identification.
#endif
#ifdef MAL_SUPPORT_WINMM
    /*UINT_PTR*/ mal_uint32 winmm;  // When creating a device, WinMM expects a Win32 UINT_PTR for device identification. In practice it's actually just a UINT.
#endif
#ifdef MAL_SUPPORT_ALSA
    char alsa[256];                 // ALSA uses a name string for identification.
#endif
#ifdef MAL_SUPPORT_PULSEAUDIO
    char pulse[256];                // PulseAudio uses a name string for identification.
#endif
#ifdef MAL_SUPPORT_JACK
    int jack;                       // JACK always uses default devices.
#endif
#ifdef MAL_SUPPORT_COREAUDIO
    char coreaudio[256];            // Core Audio uses a string for identification.
#endif
#ifdef MAL_SUPPORT_SNDIO
    char sndio[256];                // "snd/0", etc.
#endif
#ifdef MAL_SUPPORT_AUDIO4
    char audio4[256];               // "/dev/audio", etc.
#endif
#ifdef MAL_SUPPORT_OSS
    char oss[64];                   // "dev/dsp0", etc. "dev/dsp" for the default device.
#endif
#ifdef MAL_SUPPORT_OPENSL
    mal_uint32 opensl;              // OpenSL|ES uses a 32-bit unsigned integer for identification.
#endif
#ifdef MAL_SUPPORT_OPENAL
    char openal[256];               // OpenAL seems to use human-readable device names as the ID.
#endif
#ifdef MAL_SUPPORT_SDL
    int sdl;                        // SDL devices are identified with an index.
#endif
#ifdef MAL_SUPPORT_NULL
    int nullbackend;                // The null backend uses an integer for device IDs.
#endif
} mal_device_id;

typedef struct
{
    // Basic info. This is the only information guaranteed to be filled in during device enumeration.
    mal_device_id id;
    char name[256];

    // Detailed info. As much of this is filled as possible with mal_context_get_device_info(). Note that you are allowed to initialize
    // a device with settings outside of this range, but it just means the data will be converted using mini_al's data conversion
    // pipeline before sending the data to/from the device. Most programs will need to not worry about these values, but it's provided
    // here mainly for informational purposes or in the rare case that someone might find it useful.
    //
    // These will be set to 0 when returned by mal_context_enumerate_devices() or mal_context_get_devices().
    mal_uint32 formatCount;
    mal_format formats[mal_format_count];
    mal_uint32 minChannels;
    mal_uint32 maxChannels;
    mal_uint32 minSampleRate;
    mal_uint32 maxSampleRate;
} mal_device_info;

typedef struct
{
    mal_int64 counter;
} mal_timer;

typedef struct
{
    mal_format format;
    mal_uint32 channels;
    mal_uint32 sampleRate;
    mal_channel channelMap[MAL_MAX_CHANNELS];
    mal_uint32 bufferSizeInFrames;
    mal_uint32 bufferSizeInMilliseconds;
    mal_uint32 periods;
    mal_share_mode shareMode;
    mal_performance_profile performanceProfile;
    mal_recv_proc onRecvCallback;
    mal_send_proc onSendCallback;
    mal_stop_proc onStopCallback;

    struct
    {
        mal_bool32 noMMap;  // Disables MMap mode.
    } alsa;

    struct
    {
        const char* pStreamName;
    } pulse;
} mal_device_config;

typedef struct
{
    mal_log_proc onLog;
    mal_thread_priority threadPriority;

    struct
    {
        mal_bool32 useVerboseDeviceEnumeration;
    } alsa;

    struct
    {
        const char* pApplicationName;
        const char* pServerName;
        mal_bool32 tryAutoSpawn; // Enables autospawning of the PulseAudio daemon if necessary.
    } pulse;

    struct
    {
        const char* pClientName;
        mal_bool32 tryStartServer;
    } jack;
} mal_context_config;

typedef mal_bool32 (* mal_enum_devices_callback_proc)(mal_context* pContext, mal_device_type type, const mal_device_info* pInfo, void* pUserData);

struct mal_context
{
    mal_backend backend;                    // DirectSound, ALSA, etc.
    mal_context_config config;
    mal_mutex deviceEnumLock;               // Used to make mal_context_get_devices() thread safe.
    mal_mutex deviceInfoLock;               // Used to make mal_context_get_device_info() thread safe.
    mal_uint32 deviceInfoCapacity;          // Total capacity of pDeviceInfos.
    mal_uint32 playbackDeviceInfoCount;
    mal_uint32 captureDeviceInfoCount;
    mal_device_info* pDeviceInfos;          // Playback devices first, then capture.
    mal_bool32 isBackendAsynchronous : 1;   // Set when the context is initialized. Set to 1 for asynchronous backends such as Core Audio and JACK. Do not modify.

    mal_result (* onUninit             )(mal_context* pContext);
    mal_bool32 (* onDeviceIDEqual      )(mal_context* pContext, const mal_device_id* pID0, const mal_device_id* pID1);
    mal_result (* onEnumDevices        )(mal_context* pContext, mal_enum_devices_callback_proc callback, void* pUserData);    // Return false from the callback to stop enumeration.
    mal_result (* onGetDeviceInfo      )(mal_context* pContext, mal_device_type type, const mal_device_id* pDeviceID, mal_share_mode shareMode, mal_device_info* pDeviceInfo);
    mal_result (* onDeviceInit         )(mal_context* pContext, mal_device_type type, const mal_device_id* pDeviceID, const mal_device_config* pConfig, mal_device* pDevice);
    void       (* onDeviceUninit       )(mal_device* pDevice);
    mal_result (* onDeviceReinit       )(mal_device* pDevice);
    mal_result (* onDeviceStart        )(mal_device* pDevice);
    mal_result (* onDeviceStop         )(mal_device* pDevice);
    mal_result (* onDeviceBreakMainLoop)(mal_device* pDevice);
    mal_result (* onDeviceMainLoop     )(mal_device* pDevice);

    union
    {
#ifdef MAL_SUPPORT_WASAPI
        struct
        {
            int _unused;
        } wasapi;
#endif
#ifdef MAL_SUPPORT_DSOUND
        struct
        {
            /*HMODULE*/ mal_handle hDSoundDLL;
            mal_proc DirectSoundCreate;
            mal_proc DirectSoundEnumerateA;
            mal_proc DirectSoundCaptureCreate;
            mal_proc DirectSoundCaptureEnumerateA;
        } dsound;
#endif
#ifdef MAL_SUPPORT_WINMM
        struct
        {
            /*HMODULE*/ mal_handle hWinMM;
            mal_proc waveOutGetNumDevs;
            mal_proc waveOutGetDevCapsA;
            mal_proc waveOutOpen;
            mal_proc waveOutClose;
            mal_proc waveOutPrepareHeader;
            mal_proc waveOutUnprepareHeader;
            mal_proc waveOutWrite;
            mal_proc waveOutReset;
            mal_proc waveInGetNumDevs;
            mal_proc waveInGetDevCapsA;
            mal_proc waveInOpen;
            mal_proc waveInClose;
            mal_proc waveInPrepareHeader;
            mal_proc waveInUnprepareHeader;
            mal_proc waveInAddBuffer;
            mal_proc waveInStart;
            mal_proc waveInReset;
        } winmm;
#endif
#ifdef MAL_SUPPORT_ALSA
        struct
        {
            mal_handle asoundSO;
            mal_proc snd_pcm_open;
            mal_proc snd_pcm_close;
            mal_proc snd_pcm_hw_params_sizeof;
            mal_proc snd_pcm_hw_params_any;
            mal_proc snd_pcm_hw_params_set_format;
            mal_proc snd_pcm_hw_params_set_format_first;
            mal_proc snd_pcm_hw_params_get_format_mask;
            mal_proc snd_pcm_hw_params_set_channels_near;
            mal_proc snd_pcm_hw_params_set_rate_resample;
            mal_proc snd_pcm_hw_params_set_rate_near;
            mal_proc snd_pcm_hw_params_set_buffer_size_near;
            mal_proc snd_pcm_hw_params_set_periods_near;
            mal_proc snd_pcm_hw_params_set_access;
            mal_proc snd_pcm_hw_params_get_format;
            mal_proc snd_pcm_hw_params_get_channels;
            mal_proc snd_pcm_hw_params_get_channels_min;
            mal_proc snd_pcm_hw_params_get_channels_max;
            mal_proc snd_pcm_hw_params_get_rate;
            mal_proc snd_pcm_hw_params_get_rate_min;
            mal_proc snd_pcm_hw_params_get_rate_max;
            mal_proc snd_pcm_hw_params_get_buffer_size;
            mal_proc snd_pcm_hw_params_get_periods;
            mal_proc snd_pcm_hw_params_get_access;
            mal_proc snd_pcm_hw_params;
            mal_proc snd_pcm_sw_params_sizeof;
            mal_proc snd_pcm_sw_params_current;
            mal_proc snd_pcm_sw_params_set_avail_min;
            mal_proc snd_pcm_sw_params_set_start_threshold;
            mal_proc snd_pcm_sw_params;
            mal_proc snd_pcm_format_mask_sizeof;
            mal_proc snd_pcm_format_mask_test;
            mal_proc snd_pcm_get_chmap;
            mal_proc snd_pcm_prepare;
            mal_proc snd_pcm_start;
            mal_proc snd_pcm_drop;
            mal_proc snd_device_name_hint;
            mal_proc snd_device_name_get_hint;
            mal_proc snd_card_get_index;
            mal_proc snd_device_name_free_hint;
            mal_proc snd_pcm_mmap_begin;
            mal_proc snd_pcm_mmap_commit;
            mal_proc snd_pcm_recover;
            mal_proc snd_pcm_readi;
            mal_proc snd_pcm_writei;
            mal_proc snd_pcm_avail;
            mal_proc snd_pcm_avail_update;
            mal_proc snd_pcm_wait;
            mal_proc snd_pcm_info;
            mal_proc snd_pcm_info_sizeof;
            mal_proc snd_pcm_info_get_name;
            mal_proc snd_config_update_free_global;

            mal_mutex internalDeviceEnumLock;
        } alsa;
#endif
#ifdef MAL_SUPPORT_PULSEAUDIO
        struct
        {
            mal_handle pulseSO;
            mal_proc pa_mainloop_new;
            mal_proc pa_mainloop_free;
            mal_proc pa_mainloop_get_api;
            mal_proc pa_mainloop_iterate;
            mal_proc pa_mainloop_wakeup;
            mal_proc pa_context_new;
            mal_proc pa_context_unref;
            mal_proc pa_context_connect;
            mal_proc pa_context_disconnect;
            mal_proc pa_context_set_state_callback;
            mal_proc pa_context_get_state;
            mal_proc pa_context_get_sink_info_list;
            mal_proc pa_context_get_source_info_list;
            mal_proc pa_context_get_sink_info_by_name;
            mal_proc pa_context_get_source_info_by_name;
            mal_proc pa_operation_unref;
            mal_proc pa_operation_get_state;
            mal_proc pa_channel_map_init_extend;
            mal_proc pa_channel_map_valid;
            mal_proc pa_channel_map_compatible;
            mal_proc pa_stream_new;
            mal_proc pa_stream_unref;
            mal_proc pa_stream_connect_playback;
            mal_proc pa_stream_connect_record;
            mal_proc pa_stream_disconnect;
            mal_proc pa_stream_get_state;
            mal_proc pa_stream_get_sample_spec;
            mal_proc pa_stream_get_channel_map;
            mal_proc pa_stream_get_buffer_attr;
            mal_proc pa_stream_set_buffer_attr;
            mal_proc pa_stream_get_device_name;
            mal_proc pa_stream_set_write_callback;
            mal_proc pa_stream_set_read_callback;
            mal_proc pa_stream_flush;
            mal_proc pa_stream_drain;
            mal_proc pa_stream_cork;
            mal_proc pa_stream_trigger;
            mal_proc pa_stream_begin_write;
            mal_proc pa_stream_write;
            mal_proc pa_stream_peek;
            mal_proc pa_stream_drop;
        } pulse;
#endif
#ifdef MAL_SUPPORT_JACK
        struct
        {
            mal_handle jackSO;
            mal_proc jack_client_open;
            mal_proc jack_client_close;
            mal_proc jack_client_name_size;
            mal_proc jack_set_process_callback;
            mal_proc jack_set_buffer_size_callback;
            mal_proc jack_on_shutdown;
            mal_proc jack_get_sample_rate;
            mal_proc jack_get_buffer_size;
            mal_proc jack_get_ports;
            mal_proc jack_activate;
            mal_proc jack_deactivate;
            mal_proc jack_connect;
            mal_proc jack_port_register;
            mal_proc jack_port_name;
            mal_proc jack_port_get_buffer;
            mal_proc jack_free;
        } jack;
#endif
#ifdef MAL_SUPPORT_COREAUDIO
        struct
        {
            mal_handle hCoreFoundation;
            mal_proc CFStringGetCString;
            
            mal_handle hCoreAudio;
            mal_proc AudioObjectGetPropertyData;
            mal_proc AudioObjectGetPropertyDataSize;
            mal_proc AudioObjectSetPropertyData;
            mal_proc AudioObjectAddPropertyListener;
            
            mal_handle hAudioUnit;  // Could possibly be set to AudioToolbox on later versions of macOS.
            mal_proc AudioComponentFindNext;
            mal_proc AudioComponentInstanceDispose;
            mal_proc AudioComponentInstanceNew;
            mal_proc AudioOutputUnitStart;
            mal_proc AudioOutputUnitStop;
            mal_proc AudioUnitAddPropertyListener;
            mal_proc AudioUnitGetPropertyInfo;
            mal_proc AudioUnitGetProperty;
            mal_proc AudioUnitSetProperty;
            mal_proc AudioUnitInitialize;
            mal_proc AudioUnitRender;
        } coreaudio;
#endif
#ifdef MAL_SUPPORT_SNDIO
        struct
        {
            mal_handle sndioSO;
            mal_proc sio_open;
            mal_proc sio_close;
            mal_proc sio_setpar;
            mal_proc sio_getpar;
            mal_proc sio_getcap;
            mal_proc sio_start;
            mal_proc sio_stop;
            mal_proc sio_read;
            mal_proc sio_write;
            mal_proc sio_onmove;
            mal_proc sio_nfds;
            mal_proc sio_pollfd;
            mal_proc sio_revents;
            mal_proc sio_eof;
            mal_proc sio_setvol;
            mal_proc sio_onvol;
            mal_proc sio_initpar;
        } sndio;
#endif
#ifdef MAL_SUPPORT_AUDIO4
        struct
        {
            int _unused;
        } audio4;
#endif
#ifdef MAL_SUPPORT_OSS
        struct
        {
            int versionMajor;
            int versionMinor;
        } oss;
#endif
#ifdef MAL_SUPPORT_OPENSL
        struct
        {
            int _unused;
        } opensl;
#endif
#ifdef MAL_SUPPORT_OPENAL
        struct
        {
            /*HMODULE*/ mal_handle hOpenAL;     // OpenAL32.dll, etc.
            mal_proc alcCreateContext;
            mal_proc alcMakeContextCurrent;
            mal_proc alcProcessContext;
            mal_proc alcSuspendContext;
            mal_proc alcDestroyContext;
            mal_proc alcGetCurrentContext;
            mal_proc alcGetContextsDevice;
            mal_proc alcOpenDevice;
            mal_proc alcCloseDevice;
            mal_proc alcGetError;
            mal_proc alcIsExtensionPresent;
            mal_proc alcGetProcAddress;
            mal_proc alcGetEnumValue;
            mal_proc alcGetString;
            mal_proc alcGetIntegerv;
            mal_proc alcCaptureOpenDevice;
            mal_proc alcCaptureCloseDevice;
            mal_proc alcCaptureStart;
            mal_proc alcCaptureStop;
            mal_proc alcCaptureSamples;
            mal_proc alEnable;
            mal_proc alDisable;
            mal_proc alIsEnabled;
            mal_proc alGetString;
            mal_proc alGetBooleanv;
            mal_proc alGetIntegerv;
            mal_proc alGetFloatv;
            mal_proc alGetDoublev;
            mal_proc alGetBoolean;
            mal_proc alGetInteger;
            mal_proc alGetFloat;
            mal_proc alGetDouble;
            mal_proc alGetError;
            mal_proc alIsExtensionPresent;
            mal_proc alGetProcAddress;
            mal_proc alGetEnumValue;
            mal_proc alGenSources;
            mal_proc alDeleteSources;
            mal_proc alIsSource;
            mal_proc alSourcef;
            mal_proc alSource3f;
            mal_proc alSourcefv;
            mal_proc alSourcei;
            mal_proc alSource3i;
            mal_proc alSourceiv;
            mal_proc alGetSourcef;
            mal_proc alGetSource3f;
            mal_proc alGetSourcefv;
            mal_proc alGetSourcei;
            mal_proc alGetSource3i;
            mal_proc alGetSourceiv;
            mal_proc alSourcePlayv;
            mal_proc alSourceStopv;
            mal_proc alSourceRewindv;
            mal_proc alSourcePausev;
            mal_proc alSourcePlay;
            mal_proc alSourceStop;
            mal_proc alSourceRewind;
            mal_proc alSourcePause;
            mal_proc alSourceQueueBuffers;
            mal_proc alSourceUnqueueBuffers;
            mal_proc alGenBuffers;
            mal_proc alDeleteBuffers;
            mal_proc alIsBuffer;
            mal_proc alBufferData;
            mal_proc alBufferf;
            mal_proc alBuffer3f;
            mal_proc alBufferfv;
            mal_proc alBufferi;
            mal_proc alBuffer3i;
            mal_proc alBufferiv;
            mal_proc alGetBufferf;
            mal_proc alGetBuffer3f;
            mal_proc alGetBufferfv;
            mal_proc alGetBufferi;
            mal_proc alGetBuffer3i;
            mal_proc alGetBufferiv;

            mal_bool32 isEnumerationSupported : 1;
            mal_bool32 isFloat32Supported     : 1;
            mal_bool32 isMCFormatsSupported   : 1;
        } openal;
#endif
#ifdef MAL_SUPPORT_SDL
        struct
        {
            mal_handle hSDL;    // SDL
            mal_proc SDL_InitSubSystem;
            mal_proc SDL_QuitSubSystem;
            mal_proc SDL_GetNumAudioDevices;
            mal_proc SDL_GetAudioDeviceName;
            mal_proc SDL_CloseAudioDevice;
            mal_proc SDL_OpenAudioDevice;
            mal_proc SDL_PauseAudioDevice;
        } sdl;
#endif
#ifdef MAL_SUPPORT_NULL
        struct
        {
            int _unused;
        } null_backend;
#endif
    };

    union
    {
#ifdef MAL_WIN32
        struct
        {
            /*HMODULE*/ mal_handle hOle32DLL;
            mal_proc CoInitializeEx;
            mal_proc CoUninitialize;
            mal_proc CoCreateInstance;
            mal_proc CoTaskMemFree;
            mal_proc PropVariantClear;
            mal_proc StringFromGUID2;

            /*HMODULE*/ mal_handle hUser32DLL;
            mal_proc GetForegroundWindow;
            mal_proc GetDesktopWindow;

            /*HMODULE*/ mal_handle hAdvapi32DLL;
            mal_proc RegOpenKeyExA;
            mal_proc RegCloseKey;
            mal_proc RegQueryValueExA;
        } win32;
#endif
#ifdef MAL_POSIX
        struct
        {
            mal_handle pthreadSO;
            mal_proc pthread_create;
            mal_proc pthread_join;
            mal_proc pthread_mutex_init;
            mal_proc pthread_mutex_destroy;
            mal_proc pthread_mutex_lock;
            mal_proc pthread_mutex_unlock;
            mal_proc pthread_cond_init;
            mal_proc pthread_cond_destroy;
            mal_proc pthread_cond_wait;
            mal_proc pthread_cond_signal;
            mal_proc pthread_attr_init;
            mal_proc pthread_attr_destroy;
            mal_proc pthread_attr_setschedpolicy;
            mal_proc pthread_attr_getschedparam;
            mal_proc pthread_attr_setschedparam;
        } posix;
#endif
        int _unused;
    };
};

MAL_ALIGNED_STRUCT(MAL_SIMD_ALIGNMENT) mal_device
{
    mal_context* pContext;
    mal_device_type type;
    mal_format format;
    mal_uint32 channels;
    mal_uint32 sampleRate;
    mal_channel channelMap[MAL_MAX_CHANNELS];
    mal_uint32 bufferSizeInFrames;
    mal_uint32 bufferSizeInMilliseconds;
    mal_uint32 periods;
    mal_uint32 state;
    mal_recv_proc onRecv;
    mal_send_proc onSend;
    mal_stop_proc onStop;
    void* pUserData;                // Application defined data.
    char name[256];
    mal_device_config initConfig;   // The configuration passed in to mal_device_init(). Mainly used for reinitializing the backend device.
    mal_mutex lock;
    mal_event wakeupEvent;
    mal_event startEvent;
    mal_event stopEvent;
    mal_thread thread;
    mal_result workResult;          // This is set by the worker thread after it's finished doing a job.
    mal_bool32 usingDefaultFormat     : 1;
    mal_bool32 usingDefaultChannels   : 1;
    mal_bool32 usingDefaultSampleRate : 1;
    mal_bool32 usingDefaultChannelMap : 1;
    mal_bool32 usingDefaultBufferSize : 1;
    mal_bool32 usingDefaultPeriods    : 1;
    mal_bool32 exclusiveMode          : 1;
    mal_bool32 isOwnerOfContext       : 1;  // When set to true, uninitializing the device will also uninitialize the context. Set to true when NULL is passed into mal_device_init().
    mal_bool32 isDefaultDevice        : 1;  // Used to determine if the backend should try reinitializing if the default device is unplugged.
    mal_format internalFormat;
    mal_uint32 internalChannels;
    mal_uint32 internalSampleRate;
    mal_channel internalChannelMap[MAL_MAX_CHANNELS];
    mal_dsp dsp;                    // Samples run through this to convert samples to a format suitable for use by the backend.
    mal_uint32 _dspFrameCount;      // Internal use only. Used when running the device -> DSP -> client pipeline. See mal_device__on_read_from_device().
    const mal_uint8* _dspFrames;    // ^^^ AS ABOVE ^^^

    union
    {
#ifdef MAL_SUPPORT_WASAPI
        struct
        {
            /*IAudioClient**/ mal_ptr pAudioClient;
            /*IAudioRenderClient**/ mal_ptr pRenderClient;
            /*IAudioCaptureClient**/ mal_ptr pCaptureClient;
            /*IMMDeviceEnumerator**/ mal_ptr pDeviceEnumerator; /* <-- Used for IMMNotificationClient notifications. Required for detecting default device changes. */
            mal_IMMNotificationClient notificationClient;
            /*HANDLE*/ mal_handle hEvent;
            /*HANDLE*/ mal_handle hBreakEvent;  /* <-- Used to break from WaitForMultipleObjects() in the main loop. */
            mal_bool32 breakFromMainLoop;
            mal_bool32 hasDefaultDeviceChanged; /* <-- Make sure this is always a whole 32-bits because we use atomic assignments. */
        } wasapi;
#endif
#ifdef MAL_SUPPORT_DSOUND
        struct
        {
            /*LPDIRECTSOUND*/ mal_ptr pPlayback;
            /*LPDIRECTSOUNDBUFFER*/ mal_ptr pPlaybackPrimaryBuffer;
            /*LPDIRECTSOUNDBUFFER*/ mal_ptr pPlaybackBuffer;
            /*LPDIRECTSOUNDCAPTURE*/ mal_ptr pCapture;
            /*LPDIRECTSOUNDCAPTUREBUFFER*/ mal_ptr pCaptureBuffer;
            /*LPDIRECTSOUNDNOTIFY*/ mal_ptr pNotify;
            /*HANDLE*/ mal_handle pNotifyEvents[MAL_MAX_PERIODS_DSOUND];  // One event handle for each period.
            /*HANDLE*/ mal_handle hStopEvent;
            mal_uint32 lastProcessedFrame;      // This is circular.
            mal_bool32 breakFromMainLoop;
        } dsound;
#endif
#ifdef MAL_SUPPORT_WINMM
        struct
        {
            /*HWAVEOUT, HWAVEIN*/ mal_handle hDevice;
            /*HANDLE*/ mal_handle hEvent;
            mal_uint32 fragmentSizeInFrames;
            mal_uint32 fragmentSizeInBytes;
            mal_uint32 iNextHeader;             // [0,periods). Used as an index into pWAVEHDR.
            /*WAVEHDR**/ mal_uint8* pWAVEHDR;   // One instantiation for each period.
            mal_uint8* pIntermediaryBuffer;
            mal_uint8* _pHeapData;              // Used internally and is used for the heap allocated data for the intermediary buffer and the WAVEHDR structures.
            mal_bool32 breakFromMainLoop;
        } winmm;
#endif
#ifdef MAL_SUPPORT_ALSA
        struct
        {
            /*snd_pcm_t**/ mal_ptr pPCM;
            mal_bool32 isUsingMMap       : 1;
            mal_bool32 breakFromMainLoop : 1;
            void* pIntermediaryBuffer;
        } alsa;
#endif
#ifdef MAL_SUPPORT_PULSEAUDIO
        struct
        {
            /*pa_mainloop**/ mal_ptr pMainLoop;
            /*pa_mainloop_api**/ mal_ptr pAPI;
            /*pa_context**/ mal_ptr pPulseContext;
            /*pa_stream**/ mal_ptr pStream;
            /*pa_context_state*/ mal_uint32 pulseContextState;
            mal_uint32 fragmentSizeInBytes;
            mal_bool32 breakFromMainLoop : 1;
        } pulse;
#endif
#ifdef MAL_SUPPORT_JACK
        struct
        {
            /*jack_client_t**/ mal_ptr pClient;
            /*jack_port_t**/ mal_ptr pPorts[MAL_MAX_CHANNELS];
            float* pIntermediaryBuffer; // Typed as a float because JACK is always floating point.
        } jack;
#endif
#ifdef MAL_SUPPORT_COREAUDIO
        struct
        {
            mal_uint32 deviceObjectID;
            /*AudioComponent*/ mal_ptr component;   // <-- Can this be per-context?
            /*AudioUnit*/ mal_ptr audioUnit;
            /*AudioBufferList**/ mal_ptr pAudioBufferList;  // Only used for input devices.
            mal_bool32 isSwitchingDevice;   /* <-- Set to true when the default device has changed and mini_al is in the process of switching. */
        } coreaudio;
#endif
#ifdef MAL_SUPPORT_SNDIO
        struct
        {
            mal_ptr handle;
            mal_uint32 fragmentSizeInFrames;
            mal_bool32 breakFromMainLoop;
            void* pIntermediaryBuffer;
        } sndio;
#endif
#ifdef MAL_SUPPORT_AUDIO4
        struct
        {
            int fd;
            mal_uint32 fragmentSizeInFrames;
            mal_bool32 breakFromMainLoop;
            void* pIntermediaryBuffer;
        } audio4;
#endif
#ifdef MAL_SUPPORT_OSS
        struct
        {
            int fd;
            mal_uint32 fragmentSizeInFrames;
            mal_bool32 breakFromMainLoop;
            void* pIntermediaryBuffer;
        } oss;
#endif
#ifdef MAL_SUPPORT_OPENSL
        struct
        {
            /*SLObjectItf*/ mal_ptr pOutputMixObj;
            /*SLOutputMixItf*/ mal_ptr pOutputMix;
            /*SLObjectItf*/ mal_ptr pAudioPlayerObj;
            /*SLPlayItf*/ mal_ptr pAudioPlayer;
            /*SLObjectItf*/ mal_ptr pAudioRecorderObj;
            /*SLRecordItf*/ mal_ptr pAudioRecorder;
            /*SLAndroidSimpleBufferQueueItf*/ mal_ptr pBufferQueue;
            mal_uint32 periodSizeInFrames;
            mal_uint32 currentBufferIndex;
            mal_uint8* pBuffer;                 // This is malloc()'d and is used for storing audio data. Typed as mal_uint8 for easy offsetting.
        } opensl;
#endif
#ifdef MAL_SUPPORT_OPENAL
        struct
        {
            /*ALCcontext**/ mal_ptr pContextALC;
            /*ALCdevice**/ mal_ptr pDeviceALC;
            /*ALuint*/ mal_uint32 sourceAL;
            /*ALuint*/ mal_uint32 buffersAL[MAL_MAX_PERIODS_OPENAL];
            /*ALenum*/ mal_uint32 formatAL;
            mal_uint32 subBufferSizeInFrames;   // This is the size of each of the OpenAL buffers (buffersAL).
            mal_uint8* pIntermediaryBuffer;     // This is malloc()'d and is used as the destination for reading from the client. Typed as mal_uint8 for easy offsetting.
            mal_uint32 iNextBuffer;             // The next buffer to unenqueue and then re-enqueue as new data is read.
            mal_bool32 breakFromMainLoop;
        } openal;
#endif
#ifdef MAL_SUPPORT_SDL
        struct
        {
            mal_uint32 deviceID;
        } sdl;
#endif
#ifdef MAL_SUPPORT_NULL
        struct
        {
            mal_timer timer;
            mal_uint32 lastProcessedFrame;      // This is circular.
            mal_bool32 breakFromMainLoop;
            mal_uint8* pBuffer;                 // This is malloc()'d and is used as the destination for reading from the client. Typed as mal_uint8 for easy offsetting.
        } null_device;
#endif
    };
};
#if defined(_MSC_VER)
    #pragma warning(pop)
#endif

// Initializes a context.
//
// The context is used for selecting and initializing the relevant backends.
//
// Note that the location of the context cannot change throughout it's lifetime. Consider allocating
// the mal_context object with malloc() if this is an issue. The reason for this is that a pointer
// to the context is stored in the mal_device structure.
//
// <backends> is used to allow the application to prioritize backends depending on it's specific
// requirements. This can be null in which case it uses the default priority, which is as follows:
//   - WASAPI
//   - DirectSound
//   - WinMM
//   - Core Audio (Apple)
//   - sndio
//   - audio(4)
//   - OSS
//   - PulseAudio
//   - ALSA
//   - JACK
//   - OpenSL|ES
//   - OpenAL
//   - SDL
//   - Null
//
// <pConfig> is used to configure the context. Use the onLog config to set a callback for whenever a
// log message is posted. The priority of the worker thread can be set with the threadPriority config.
//
// It is recommended that only a single context is active at any given time because it's a bulky data
// structure which performs run-time linking for the relevant backends every time it's initialized.
//
// Return Value:
//   MAL_SUCCESS if successful; any other error code otherwise.
//
// Thread Safety: UNSAFE
mal_result mal_context_init(const mal_backend backends[], mal_uint32 backendCount, const mal_context_config* pConfig, mal_context* pContext);

// Uninitializes a context.
//
// Results are undefined if you call this while any device created by this context is still active.
//
// Return Value:
//   MAL_SUCCESS if successful; any other error code otherwise.
//
// Thread Safety: UNSAFE
mal_result mal_context_uninit(mal_context* pContext);

// Enumerates over every device (both playback and capture).
//
// This is a lower-level enumeration function to the easier to use mal_context_get_devices(). Use
// mal_context_enumerate_devices() if you would rather not incur an internal heap allocation, or
// it simply suits your code better.
//
// Do _not_ assume the first enumerated device of a given type is the default device.
//
// Some backends and platforms may only support default playback and capture devices.
//
// Note that this only retrieves the ID and name/description of the device. The reason for only
// retrieving basic information is that it would otherwise require opening the backend device in
// order to probe it for more detailed information which can be inefficient. Consider using
// mal_context_get_device_info() for this, but don't call it from within the enumeration callback.
//
// In general, you should not do anything complicated from within the callback. In particular, do
// not try initializing a device from within the callback.
//
// Consider using mal_context_get_devices() for a simpler and safer API, albeit at the expense of
// an internal heap allocation.
//
// Returning false from the callback will stop enumeration. Returning true will continue enumeration.
//
// Return Value:
//   MAL_SUCCESS if successful; any other error code otherwise.
//
// Thread Safety: SAFE
//   This is guarded using a simple mutex lock.
mal_result mal_context_enumerate_devices(mal_context* pContext, mal_enum_devices_callback_proc callback, void* pUserData);

// Retrieves basic information about every active playback and/or capture device.
//
// You can pass in NULL for the playback or capture lists in which case they'll be ignored.
//
// It is _not_ safe to assume the first device in the list is the default device.
//
// The returned pointers will become invalid upon the next call this this function, or when the
// context is uninitialized. Do not free the returned pointers.
//
// This function follows the same enumeration rules as mal_context_enumerate_devices(). See
// documentation for mal_context_enumerate_devices() for more information.
//
// Return Value:
//   MAL_SUCCESS if successful; any other error code otherwise.
//
// Thread Safety: SAFE
//   Since each call to this function invalidates the pointers from the previous call, you
//   should not be calling this simultaneously across multiple threads. Instead, you need to
//   make a copy of the returned data with your own higher level synchronization.
mal_result mal_context_get_devices(mal_context* pContext, mal_device_info** ppPlaybackDeviceInfos, mal_uint32* pPlaybackDeviceCount, mal_device_info** ppCaptureDeviceInfos, mal_uint32* pCaptureDeviceCount);

// Retrieves information about a device with the given ID.
//
// Do _not_ call this from within the mal_context_enumerate_devices() callback.
//
// It's possible for a device to have different information and capabilities depending on wether or
// not it's opened in shared or exclusive mode. For example, in shared mode, WASAPI always uses
// floating point samples for mixing, but in exclusive mode it can be anything. Therefore, this
// function allows you to specify which share mode you want information for. Note that not all
// backends and devices support shared or exclusive mode, in which case this function will fail
// if the requested share mode is unsupported.
//
// This leaves pDeviceInfo unmodified in the result of an error.
//
// Return Value:
//   MAL_SUCCESS if successful; any other error code otherwise.
//
// Thread Safety: SAFE
//   This is guarded using a simple mutex lock.
mal_result mal_context_get_device_info(mal_context* pContext, mal_device_type type, const mal_device_id* pDeviceID, mal_share_mode shareMode, mal_device_info* pDeviceInfo);

// Initializes a device.
//
// The context can be null in which case it uses the default. This is equivalent to passing in a
// context that was initialized like so:
//
//     mal_context_init(NULL, 0, NULL, &context);
//
// Do not pass in null for the context if you are needing to open multiple devices. You can,
// however, use null when initializing the first device, and then use device.pContext for the
// initialization of other devices.
//
// The device ID (pDeviceID) can be null, in which case the default device is used. Otherwise, you
// can retrieve the ID by calling mal_context_get_devices() and using the ID from the returned data.
// Set pDeviceID to NULL to use the default device. Do _not_ rely on the first device ID returned
// by mal_context_enumerate_devices() or mal_context_get_devices() to be the default device.
//
// The device's configuration is controlled with pConfig. This allows you to configure the sample
// format, channel count, sample rate, etc. Before calling mal_device_init(), you will most likely
// want to initialize a mal_device_config object using mal_device_config_init(),
// mal_device_config_init_playback(), etc. You can also pass in NULL for the device config in
// which case it will use defaults, but will require you to call mal_device_set_recv_callback() or
// mal_device_set_send_callback() before starting the device.
//
// Passing in 0 to any property in pConfig will force the use of a default value. In the case of
// sample format, channel count, sample rate and channel map it will default to the values used by
// the backend's internal device. For the size of the buffer you can set bufferSizeInFrames or
// bufferSizeInMilliseconds (if both are set it will prioritize bufferSizeInFrames). If both are
// set to zero, it will default to MAL_BASE_BUFFER_SIZE_IN_MILLISECONDS_LOW_LATENCY or
// MAL_BASE_BUFFER_SIZE_IN_MILLISECONDS_CONSERVATIVE, depending on whether or not performanceProfile
// is set to mal_performance_profile_low_latency or mal_performance_profile_conservative.
//
// When sending or receiving data to/from a device, mini_al will internally perform a format
// conversion to convert between the format specified by pConfig and the format used internally by
// the backend. If you pass in NULL for pConfig or 0 for the sample format, channel count,
// sample rate _and_ channel map, data transmission will run on an optimized pass-through fast path.
//
// The <periods> property controls how frequently the background thread is woken to check for more
// data. It's tied to the buffer size, so as an example, if your buffer size is equivalent to 10
// milliseconds and you have 2 periods, the CPU will wake up approximately every 5 milliseconds.
//
// When compiling for UWP you must ensure you call this function on the main UI thread because the
// operating system may need to present the user with a message asking for permissions. Please refer
// to the official documentation for ActivateAudioInterfaceAsync() for more information.
//
// Return Value:
//   MAL_SUCCESS if successful; any other error code otherwise.
//
// Thread Safety: UNSAFE
//   It is not safe to call this function simultaneously for different devices because some backends
//   depend on and mutate global state (such as OpenSL|ES). The same applies to calling this at the
//   same time as mal_device_uninit().
mal_result mal_device_init(mal_context* pContext, mal_device_type type, mal_device_id* pDeviceID, const mal_device_config* pConfig, void* pUserData, mal_device* pDevice);

// Initializes a device without a context, with extra parameters for controlling the configuration
// of the internal self-managed context.
//
// See mal_device_init() and mal_context_init().
mal_result mal_device_init_ex(const mal_backend backends[], mal_uint32 backendCount, const mal_context_config* pContextConfig, mal_device_type type, mal_device_id* pDeviceID, const mal_device_config* pConfig, void* pUserData, mal_device* pDevice);

// Uninitializes a device.
//
// This will explicitly stop the device. You do not need to call mal_device_stop() beforehand, but it's
// harmless if you do.
//
// Return Value:
//   MAL_SUCCESS if successful; any other error code otherwise.
//
// Thread Safety: UNSAFE
//   As soon as this API is called the device should be considered undefined. All bets are off if you
//   try using the device at the same time as uninitializing it.
void mal_device_uninit(mal_device* pDevice);

// Sets the callback to use when the application has received data from the device.
//
// Thread Safety: SAFE
//   This API is implemented as a simple atomic assignment.
//
// DEPRECATED. Set this when the device is initialized with mal_device_init*().
void mal_device_set_recv_callback(mal_device* pDevice, mal_recv_proc proc);

// Sets the callback to use when the application needs to send data to the device for playback.
//
// Note that the implementation of this callback must copy over as many samples as is available. The
// return value specifies how many samples were written to the output buffer. The backend will fill
// any leftover samples with silence.
//
// Thread Safety: SAFE
//   This API is implemented as a simple atomic assignment.
//
// DEPRECATED. Set this when the device is initialized with mal_device_init*().
void mal_device_set_send_callback(mal_device* pDevice, mal_send_proc proc);

// Sets the callback to use when the device has stopped, either explicitly or as a result of an error.
//
// Thread Safety: SAFE
//   This API is implemented as a simple atomic assignment.
void mal_device_set_stop_callback(mal_device* pDevice, mal_stop_proc proc);

// Activates the device. For playback devices this begins playback. For capture devices it begins
// recording.
//
// For a playback device, this will retrieve an initial chunk of audio data from the client before
// returning. The reason for this is to ensure there is valid audio data in the buffer, which needs
// to be done _before_ the device begins playback.
//
// This API waits until the backend device has been started for real by the worker thread. It also
// waits on a mutex for thread-safety.
//
// Return Value:
//   - MAL_SUCCESS if successful; any other error code otherwise.
//   - MAL_INVALID_ARGS
//       One or more of the input arguments is invalid.
//   - MAL_DEVICE_NOT_INITIALIZED
//       The device is not currently or was never initialized.
//   - MAL_DEVICE_BUSY
//       The device is in the process of stopping. This will only happen if mal_device_start() and
//       mal_device_stop() is called simultaneous on separate threads. This will never be returned in
//       single-threaded applications.
//   - MAL_DEVICE_ALREADY_STARTING
//       The device is already in the process of starting. This will never be returned in single-threaded
//       applications.
//   - MAL_DEVICE_ALREADY_STARTED
//       The device is already started.
//   - MAL_FAILED_TO_READ_DATA_FROM_CLIENT
//       Failed to read the initial chunk of audio data from the client. This initial chunk of data is
//       required so that the device has valid audio data as soon as it starts playing. This will never
//       be returned for capture devices.
//   - MAL_FAILED_TO_START_BACKEND_DEVICE
//       There was a backend-specific error starting the device.
//
// Thread Safety: SAFE
mal_result mal_device_start(mal_device* pDevice);

// Puts the device to sleep, but does not uninitialize it. Use mal_device_start() to start it up again.
//
// This API needs to wait on the worker thread to stop the backend device properly before returning. It
// also waits on a mutex for thread-safety. In addition, some backends need to wait for the device to
// finish playback/recording of the current fragment which can take some time (usually proportionate to
// the buffer size that was specified at initialization time).
//
// Return Value:
//   - MAL_SUCCESS if successful; any other error code otherwise.
//   - MAL_INVALID_ARGS
//       One or more of the input arguments is invalid.
//   - MAL_DEVICE_NOT_INITIALIZED
//       The device is not currently or was never initialized.
//   - MAL_DEVICE_BUSY
//       The device is in the process of starting. This will only happen if mal_device_start() and
//       mal_device_stop() is called simultaneous on separate threads. This will never be returned in
//       single-threaded applications.
//   - MAL_DEVICE_ALREADY_STOPPING
//       The device is already in the process of stopping. This will never be returned in single-threaded
//       applications.
//   - MAL_DEVICE_ALREADY_STOPPED
//       The device is already stopped.
//   - MAL_FAILED_TO_STOP_BACKEND_DEVICE
//       There was a backend-specific error stopping the device.
//
// Thread Safety: SAFE
mal_result mal_device_stop(mal_device* pDevice);

// Determines whether or not the device is started.
//
// This is implemented as a simple accessor.
//
// Return Value:
//   True if the device is started, false otherwise.
//
// Thread Safety: SAFE
//   If another thread calls mal_device_start() or mal_device_stop() at this same time as this function
//   is called, there's a very small chance the return value will be out of sync.
mal_bool32 mal_device_is_started(mal_device* pDevice);

// Retrieves the size of the buffer in bytes for the given device.
//
// This API is efficient and is implemented with just a few 32-bit integer multiplications.
//
// Thread Safety: SAFE
//   This is calculated from constant values which are set at initialization time and never change.
mal_uint32 mal_device_get_buffer_size_in_bytes(mal_device* pDevice);


// Helper function for initializing a mal_context_config object.
mal_context_config mal_context_config_init(mal_log_proc onLog);

// Initializes a default device config.
//
// A default configuration will configure the device such that the format, channel count, sample rate and channel map are
// the same as the backend's internal configuration. This means the application loses explicit control of these properties,
// but in return gets an optimized fast path for data transmission since mini_al will be releived of all format conversion
// duties. You will not typically want to use default configurations unless you have some specific low-latency requirements.
//
// mal_device_config_init(), mal_device_config_init_playback(), etc. will allow you to explicitly set the sample format,
// channel count, etc.
mal_device_config mal_device_config_init_default(void);
mal_device_config mal_device_config_init_default_capture(mal_recv_proc onRecvCallback);
mal_device_config mal_device_config_init_default_playback(mal_send_proc onSendCallback);

// Helper function for initializing a mal_device_config object.
//
// This is just a helper API, and as such the returned object can be safely modified as needed.
//
// The default channel mapping is based on the channel count, as per the table below. Note that these
// can be freely changed after this function returns if you are needing something in particular.
//
// |---------------|------------------------------|
// | Channel Count | Mapping                      |
// |---------------|------------------------------|
// | 1 (Mono)      | 0: MAL_CHANNEL_MONO          |
// |---------------|------------------------------|
// | 2 (Stereo)    | 0: MAL_CHANNEL_FRONT_LEFT    |
// |               | 1: MAL_CHANNEL_FRONT_RIGHT   |
// |---------------|------------------------------|
// | 3             | 0: MAL_CHANNEL_FRONT_LEFT    |
// |               | 1: MAL_CHANNEL_FRONT_RIGHT   |
// |               | 2: MAL_CHANNEL_FRONT_CENTER  |
// |---------------|------------------------------|
// | 4 (Surround)  | 0: MAL_CHANNEL_FRONT_LEFT    |
// |               | 1: MAL_CHANNEL_FRONT_RIGHT   |
// |               | 2: MAL_CHANNEL_FRONT_CENTER  |
// |               | 3: MAL_CHANNEL_BACK_CENTER   |
// |---------------|------------------------------|
// | 5             | 0: MAL_CHANNEL_FRONT_LEFT    |
// |               | 1: MAL_CHANNEL_FRONT_RIGHT   |
// |               | 2: MAL_CHANNEL_FRONT_CENTER  |
// |               | 3: MAL_CHANNEL_BACK_LEFT     |
// |               | 4: MAL_CHANNEL_BACK_RIGHT    |
// |---------------|------------------------------|
// | 6 (5.1)       | 0: MAL_CHANNEL_FRONT_LEFT    |
// |               | 1: MAL_CHANNEL_FRONT_RIGHT   |
// |               | 2: MAL_CHANNEL_FRONT_CENTER  |
// |               | 3: MAL_CHANNEL_LFE           |
// |               | 4: MAL_CHANNEL_SIDE_LEFT     |
// |               | 5: MAL_CHANNEL_SIDE_RIGHT    |
// |---------------|------------------------------|
// | 7             | 0: MAL_CHANNEL_FRONT_LEFT    |
// |               | 1: MAL_CHANNEL_FRONT_RIGHT   |
// |               | 2: MAL_CHANNEL_FRONT_CENTER  |
// |               | 3: MAL_CHANNEL_LFE           |
// |               | 4: MAL_CHANNEL_BACK_CENTER   |
// |               | 4: MAL_CHANNEL_SIDE_LEFT     |
// |               | 5: MAL_CHANNEL_SIDE_RIGHT    |
// |---------------|------------------------------|
// | 8 (7.1)       | 0: MAL_CHANNEL_FRONT_LEFT    |
// |               | 1: MAL_CHANNEL_FRONT_RIGHT   |
// |               | 2: MAL_CHANNEL_FRONT_CENTER  |
// |               | 3: MAL_CHANNEL_LFE           |
// |               | 4: MAL_CHANNEL_BACK_LEFT     |
// |               | 5: MAL_CHANNEL_BACK_RIGHT    |
// |               | 6: MAL_CHANNEL_SIDE_LEFT     |
// |               | 7: MAL_CHANNEL_SIDE_RIGHT    |
// |---------------|------------------------------|
// | Other         | All channels set to 0. This  |
// |               | is equivalent to the same    |
// |               | mapping as the device.       |
// |---------------|------------------------------|
//
// Thread Safety: SAFE
//
// Efficiency: HIGH
//   This just returns a stack allocated object and consists of just a few assignments.
mal_device_config mal_device_config_init_ex(mal_format format, mal_uint32 channels, mal_uint32 sampleRate, mal_channel channelMap[MAL_MAX_CHANNELS], mal_recv_proc onRecvCallback, mal_send_proc onSendCallback);

// A simplified version of mal_device_config_init_ex().
static MAL_INLINE mal_device_config mal_device_config_init(mal_format format, mal_uint32 channels, mal_uint32 sampleRate, mal_recv_proc onRecvCallback, mal_send_proc onSendCallback) { return mal_device_config_init_ex(format, channels, sampleRate, NULL, onRecvCallback, onSendCallback); }

// A simplified version of mal_device_config_init() for capture devices.
static MAL_INLINE mal_device_config mal_device_config_init_capture_ex(mal_format format, mal_uint32 channels, mal_uint32 sampleRate, mal_channel channelMap[MAL_MAX_CHANNELS], mal_recv_proc onRecvCallback) { return mal_device_config_init_ex(format, channels, sampleRate, channelMap, onRecvCallback, NULL); }
static MAL_INLINE mal_device_config mal_device_config_init_capture(mal_format format, mal_uint32 channels, mal_uint32 sampleRate, mal_recv_proc onRecvCallback) { return mal_device_config_init_capture_ex(format, channels, sampleRate, NULL, onRecvCallback); }

// A simplified version of mal_device_config_init() for playback devices.
static MAL_INLINE mal_device_config mal_device_config_init_playback_ex(mal_format format, mal_uint32 channels, mal_uint32 sampleRate, mal_channel channelMap[MAL_MAX_CHANNELS], mal_send_proc onSendCallback) { return mal_device_config_init_ex(format, channels, sampleRate, channelMap, NULL, onSendCallback); }
static MAL_INLINE mal_device_config mal_device_config_init_playback(mal_format format, mal_uint32 channels, mal_uint32 sampleRate, mal_send_proc onSendCallback) { return mal_device_config_init_playback_ex(format, channels, sampleRate, NULL, onSendCallback); }



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utiltities
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Creates a mutex.
//
// A mutex must be created from a valid context. A mutex is initially unlocked.
mal_result mal_mutex_init(mal_context* pContext, mal_mutex* pMutex);

// Deletes a mutex.
void mal_mutex_uninit(mal_mutex* pMutex);

// Locks a mutex with an infinite timeout.
void mal_mutex_lock(mal_mutex* pMutex);

// Unlocks a mutex.
void mal_mutex_unlock(mal_mutex* pMutex);


// Retrieves a friendly name for a backend.
const char* mal_get_backend_name(mal_backend backend);

// Adjust buffer size based on a scaling factor.
//
// This just multiplies the base size by the scaling factor, making sure it's a size of at least 1.
mal_uint32 mal_scale_buffer_size(mal_uint32 baseBufferSize, float scale);

// Calculates a buffer size in milliseconds from the specified number of frames and sample rate.
mal_uint32 mal_calculate_buffer_size_in_milliseconds_from_frames(mal_uint32 bufferSizeInFrames, mal_uint32 sampleRate);

// Calculates a buffer size in frames from the specified number of milliseconds and sample rate.
mal_uint32 mal_calculate_buffer_size_in_frames_from_milliseconds(mal_uint32 bufferSizeInMilliseconds, mal_uint32 sampleRate);

// Retrieves the default buffer size in milliseconds based on the specified performance profile.
mal_uint32 mal_get_default_buffer_size_in_milliseconds(mal_performance_profile performanceProfile);

// Calculates a buffer size in frames for the specified performance profile and scale factor.
mal_uint32 mal_get_default_buffer_size_in_frames(mal_performance_profile performanceProfile, mal_uint32 sampleRate);

#endif  // MAL_NO_DEVICE_IO




//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Decoding
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef MAL_NO_DECODING

typedef struct mal_decoder mal_decoder;

typedef enum
{
    mal_seek_origin_start,
    mal_seek_origin_current
} mal_seek_origin;

typedef size_t     (* mal_decoder_read_proc)         (mal_decoder* pDecoder, void* pBufferOut, size_t bytesToRead); // Returns the number of bytes read.
typedef mal_bool32 (* mal_decoder_seek_proc)         (mal_decoder* pDecoder, int byteOffset, mal_seek_origin origin);
typedef mal_result (* mal_decoder_seek_to_frame_proc)(mal_decoder* pDecoder, mal_uint64 frameIndex);
typedef mal_result (* mal_decoder_uninit_proc)       (mal_decoder* pDecoder);

typedef struct
{
    mal_format format;      // Set to 0 or mal_format_unknown to use the stream's internal format.
    mal_uint32 channels;    // Set to 0 to use the stream's internal channels.
    mal_uint32 sampleRate;  // Set to 0 to use the stream's internal sample rate.
    mal_channel channelMap[MAL_MAX_CHANNELS];
    mal_channel_mix_mode channelMixMode;
    mal_dither_mode ditherMode;
    mal_src_algorithm srcAlgorithm;
    union
    {
        mal_src_config_sinc sinc;
    } src;
} mal_decoder_config;

struct mal_decoder
{
    mal_decoder_read_proc onRead;
    mal_decoder_seek_proc onSeek;
    void* pUserData;
    mal_format  internalFormat;
    mal_uint32  internalChannels;
    mal_uint32  internalSampleRate;
    mal_channel internalChannelMap[MAL_MAX_CHANNELS];
    mal_format  outputFormat;
    mal_uint32  outputChannels;
    mal_uint32  outputSampleRate;
    mal_channel outputChannelMap[MAL_MAX_CHANNELS];
    mal_dsp dsp;                // <-- Format conversion is achieved by running frames through this.
    mal_decoder_seek_to_frame_proc onSeekToFrame;
    mal_decoder_uninit_proc onUninit;
    void* pInternalDecoder;     // <-- The drwav/drflac/stb_vorbis/etc. objects.
    struct
    {
        const mal_uint8* pData;
        size_t dataSize;
        size_t currentReadPos;
    } memory;   // Only used for decoders that were opened against a block of memory.
};

mal_decoder_config mal_decoder_config_init(mal_format outputFormat, mal_uint32 outputChannels, mal_uint32 outputSampleRate);

mal_result mal_decoder_init(mal_decoder_read_proc onRead, mal_decoder_seek_proc onSeek, void* pUserData, const mal_decoder_config* pConfig, mal_decoder* pDecoder);
mal_result mal_decoder_init_wav(mal_decoder_read_proc onRead, mal_decoder_seek_proc onSeek, void* pUserData, const mal_decoder_config* pConfig, mal_decoder* pDecoder);
mal_result mal_decoder_init_flac(mal_decoder_read_proc onRead, mal_decoder_seek_proc onSeek, void* pUserData, const mal_decoder_config* pConfig, mal_decoder* pDecoder);
mal_result mal_decoder_init_vorbis(mal_decoder_read_proc onRead, mal_decoder_seek_proc onSeek, void* pUserData, const mal_decoder_config* pConfig, mal_decoder* pDecoder);
mal_result mal_decoder_init_mp3(mal_decoder_read_proc onRead, mal_decoder_seek_proc onSeek, void* pUserData, const mal_decoder_config* pConfig, mal_decoder* pDecoder);
mal_result mal_decoder_init_raw(mal_decoder_read_proc onRead, mal_decoder_seek_proc onSeek, void* pUserData, const mal_decoder_config* pConfigIn, const mal_decoder_config* pConfigOut, mal_decoder* pDecoder);

mal_result mal_decoder_init_memory(const void* pData, size_t dataSize, const mal_decoder_config* pConfig, mal_decoder* pDecoder);
mal_result mal_decoder_init_memory_wav(const void* pData, size_t dataSize, const mal_decoder_config* pConfig, mal_decoder* pDecoder);
mal_result mal_decoder_init_memory_flac(const void* pData, size_t dataSize, const mal_decoder_config* pConfig, mal_decoder* pDecoder);
mal_result mal_decoder_init_memory_vorbis(const void* pData, size_t dataSize, const mal_decoder_config* pConfig, mal_decoder* pDecoder);
mal_result mal_decoder_init_memory_mp3(const void* pData, size_t dataSize, const mal_decoder_config* pConfig, mal_decoder* pDecoder);
mal_result mal_decoder_init_memory_raw(const void* pData, size_t dataSize, const mal_decoder_config* pConfigIn, const mal_decoder_config* pConfigOut, mal_decoder* pDecoder);

#ifndef MAL_NO_STDIO
mal_result mal_decoder_init_file(const char* pFilePath, const mal_decoder_config* pConfig, mal_decoder* pDecoder);
mal_result mal_decoder_init_file_wav(const char* pFilePath, const mal_decoder_config* pConfig, mal_decoder* pDecoder);
#endif

mal_result mal_decoder_uninit(mal_decoder* pDecoder);

mal_uint64 mal_decoder_read(mal_decoder* pDecoder, mal_uint64 frameCount, void* pFramesOut);
mal_result mal_decoder_seek_to_frame(mal_decoder* pDecoder, mal_uint64 frameIndex);


// Helper for opening and decoding a file into a heap allocated block of memory. Free the returned pointer with mal_free(). On input,
// pConfig should be set to what you want. On output it will be set to what you got.
#ifndef MAL_NO_STDIO
mal_result mal_decode_file(const char* pFilePath, mal_decoder_config* pConfig, mal_uint64* pFrameCountOut, void** ppDataOut);
#endif
mal_result mal_decode_memory(const void* pData, size_t dataSize, mal_decoder_config* pConfig, mal_uint64* pFrameCountOut, void** ppDataOut);

#endif  // MAL_NO_DECODING


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Generation
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct
{
    double amplitude;
    double periodsPerSecond;
    double delta;
    double time;
} mal_sine_wave;

mal_result mal_sine_wave_init(double amplitude, double period, mal_uint32 sampleRate, mal_sine_wave* pSineWave);
mal_uint64 mal_sine_wave_read(mal_sine_wave* pSineWave, mal_uint64 count, float* pSamples);
mal_uint64 mal_sine_wave_read_ex(mal_sine_wave* pSineWave, mal_uint64 frameCount, mal_uint32 channels, mal_stream_layout layout, float** ppFrames);


#ifdef __cplusplus
}
#endif
#endif  //mini_al_h


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// IMPLEMENTATION
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if defined(MINI_AL_IMPLEMENTATION) || defined(MAL_IMPLEMENTATION)
#include <assert.h>
#include <limits.h> // For INT_MAX
#include <math.h>   // sin(), etc.

#if defined(MAL_DEBUG_OUTPUT)
#include <stdio.h>  // for printf() for debug output
#endif

#ifdef MAL_WIN32
#include <windows.h>
#else
#include <stdlib.h> // For malloc()/free()
#include <string.h> // For memset()
#endif

#if defined(MAL_APPLE) && (__MAC_OS_X_VERSION_MIN_REQUIRED < 101200)
#include <mach/mach_time.h> // For mach_absolute_time()
#endif

#ifdef MAL_POSIX
#include <unistd.h>
#include <dlfcn.h>
#endif

#if !defined(MAL_64BIT) && !defined(MAL_32BIT)
#ifdef _WIN32
#ifdef _WIN64
#define MAL_64BIT
#else
#define MAL_32BIT
#endif
#endif
#endif

#if !defined(MAL_64BIT) && !defined(MAL_32BIT)
#ifdef __GNUC__
#ifdef __LP64__
#define MAL_64BIT
#else
#define MAL_32BIT
#endif
#endif
#endif

#if !defined(MAL_64BIT) && !defined(MAL_32BIT)
#include <stdint.h>
#if INTPTR_MAX == INT64_MAX
#define MAL_64BIT
#else
#define MAL_32BIT
#endif
#endif

// Architecture Detection
#if defined(__x86_64__) || defined(_M_X64)
#define MAL_X64
#elif defined(__i386) || defined(_M_IX86)
#define MAL_X86
#elif defined(__arm__) || defined(_M_ARM)
#define MAL_ARM
#endif

// Cannot currently support AVX-512 if AVX is disabled.
#if !defined(MAL_NO_AVX512) && defined(MAL_NO_AVX2)
#define MAL_NO_AVX512
#endif

// Intrinsics Support
#if defined(MAL_X64) || defined(MAL_X86)
    #if defined(_MSC_VER) && !defined(__clang__)
        // MSVC.
        #if !defined(MAL_NO_SSE2)   // Assume all MSVC compilers support SSE2 intrinsics.
            #define MAL_SUPPORT_SSE2
        #endif
        //#if _MSC_VER >= 1600 && !defined(MAL_NO_AVX)    // 2010
        //    #define MAL_SUPPORT_AVX
        //#endif
        #if _MSC_VER >= 1700 && !defined(MAL_NO_AVX2)   // 2012
            #define MAL_SUPPORT_AVX2
        #endif
        #if _MSC_VER >= 1910 && !defined(MAL_NO_AVX512) // 2017
            #define MAL_SUPPORT_AVX512
        #endif
    #else
        // Assume GNUC-style.
        #if defined(__SSE2__) && !defined(MAL_NO_SSE2)
            #define MAL_SUPPORT_SSE2
        #endif
        //#if defined(__AVX__) && !defined(MAL_NO_AVX)
        //    #define MAL_SUPPORT_AVX
        //#endif
        #if defined(__AVX2__) && !defined(MAL_NO_AVX2)
            #define MAL_SUPPORT_AVX2
        #endif
        #if defined(__AVX512F__) && !defined(MAL_NO_AVX512)
            #define MAL_SUPPORT_AVX512
        #endif
    #endif

    // If at this point we still haven't determined compiler support for the intrinsics just fall back to __has_include.
    #if !defined(__GNUC__) && !defined(__clang__) && defined(__has_include)
        #if !defined(MAL_SUPPORT_SSE2)   && !defined(MAL_NO_SSE2)   && __has_include(<emmintrin.h>)
            #define MAL_SUPPORT_SSE2
        #endif
        //#if !defined(MAL_SUPPORT_AVX)    && !defined(MAL_NO_AVX)    && __has_include(<immintrin.h>)
        //    #define MAL_SUPPORT_AVX
        //#endif
        #if !defined(MAL_SUPPORT_AVX2)   && !defined(MAL_NO_AVX2)   && __has_include(<immintrin.h>)
            #define MAL_SUPPORT_AVX2
        #endif
        #if !defined(MAL_SUPPORT_AVX512) && !defined(MAL_NO_AVX512) && __has_include(<zmmintrin.h>)
            #define MAL_SUPPORT_AVX512
        #endif
    #endif

    #if defined(MAL_SUPPORT_AVX512)
        #include <immintrin.h>  // Not a mistake. Intentionally including <immintrin.h> instead of <zmmintrin.h> because otherwise the compiler will complain.
    #elif defined(MAL_SUPPORT_AVX2) || defined(MAL_SUPPORT_AVX)
        #include <immintrin.h>
    #elif defined(MAL_SUPPORT_SSE2)
        #include <emmintrin.h>
    #endif
#endif

#if defined(MAL_ARM)
    #if !defined(MAL_NO_NEON) && (defined(__ARM_NEON) || defined(__aarch64__) || defined(_M_ARM64))
        #define MAL_SUPPORT_NEON
    #endif

    // Fall back to looking for the #include file.
    #if !defined(__GNUC__) && !defined(__clang__) && defined(__has_include)
        #if !defined(MAL_SUPPORT_NEON) && !defined(MAL_NO_NEON) && __has_include(<arm_neon.h>)
            #define MAL_SUPPORT_NEON
        #endif
    #endif

    #if defined(MAL_SUPPORT_NEON)
        #include <arm_neon.h>
    #endif
#endif

#if defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable:4752)   // found Intel(R) Advanced Vector Extensions; consider using /arch:AVX
#endif

#if defined(MAL_X64) || defined(MAL_X86)
    #if defined(_MSC_VER) && !defined(__clang__)
        #if _MSC_VER >= 1400
            #include <intrin.h>
            static MAL_INLINE void mal_cpuid(int info[4], int fid)
            {
                __cpuid(info, fid);
            }
        #else
            #define MAL_NO_CPUID
        #endif

        #if _MSC_VER >= 1600
            static MAL_INLINE unsigned __int64 mal_xgetbv(int reg)
            {
                return _xgetbv(reg);
            }
        #else
            #define MAL_NO_XGETBV
        #endif
    #elif (defined(__GNUC__) || defined(__clang__)) && !defined(MAL_ANDROID)
        static MAL_INLINE void mal_cpuid(int info[4], int fid)
        {
            // It looks like the -fPIC option uses the ebx register which GCC complains about. We can work around this by just using a different register, the
            // specific register of which I'm letting the compiler decide on. The "k" prefix is used to specify a 32-bit register. The {...} syntax is for
            // supporting different assembly dialects.
            //
            // What's basically happening is that we're saving and restoring the ebx register manually.
            #if defined(DRFLAC_X86) && defined(__PIC__)
                __asm__ __volatile__ (
                    "xchg{l} {%%}ebx, %k1;"
                    "cpuid;"
                    "xchg{l} {%%}ebx, %k1;"
                    : "=a"(info[0]), "=&r"(info[1]), "=c"(info[2]), "=d"(info[3]) : "a"(fid), "c"(0)
                );
            #else
                __asm__ __volatile__ (
                    "cpuid" : "=a"(info[0]), "=b"(info[1]), "=c"(info[2]), "=d"(info[3]) : "a"(fid), "c"(0)
                );
            #endif
        }

        static MAL_INLINE unsigned long long mal_xgetbv(int reg)
        {
            unsigned int hi;
            unsigned int lo;

            __asm__ __volatile__ (
                "xgetbv" : "=a"(lo), "=d"(hi) : "c"(reg)
            );

            return ((unsigned long long)hi << 32ULL) | (unsigned long long)lo;
        }
    #else
        #define MAL_NO_CPUID
        #define MAL_NO_XGETBV
    #endif
#else
    #define MAL_NO_CPUID
    #define MAL_NO_XGETBV
#endif

static MAL_INLINE mal_bool32 mal_has_sse2()
{
#if defined(MAL_SUPPORT_SSE2)
    #if (defined(MAL_X64) || defined(MAL_X86)) && !defined(MAL_NO_SSE2)
        #if defined(MAL_X64)
            return MAL_TRUE;    // 64-bit targets always support SSE2.
        #elif (defined(_M_IX86_FP) && _M_IX86_FP == 2) || defined(__SSE2__)
            return MAL_TRUE;    // If the compiler is allowed to freely generate SSE2 code we can assume support.
        #else
            #if defined(MAL_NO_CPUID)
                return MAL_FALSE;
            #else
                int info[4];
                mal_cpuid(info, 1);
                return (info[3] & (1 << 26)) != 0;
            #endif
        #endif
    #else
        return MAL_FALSE;       // SSE2 is only supported on x86 and x64 architectures.
    #endif
#else
    return MAL_FALSE;           // No compiler support.
#endif
}

#if 0
static MAL_INLINE mal_bool32 mal_has_avx()
{
#if defined(MAL_SUPPORT_AVX)
    #if (defined(MAL_X64) || defined(MAL_X86)) && !defined(MAL_NO_AVX)
        #if defined(_AVX_) || defined(__AVX__)
            return MAL_TRUE;    // If the compiler is allowed to freely generate AVX code we can assume support.
        #else
            // AVX requires both CPU and OS support.
            #if defined(MAL_NO_CPUID) || defined(MAL_NO_XGETBV)
                return MAL_FALSE;
            #else
                int info[4];
                mal_cpuid(info, 1);
                if (((info[2] & (1 << 27)) != 0) && ((info[2] & (1 << 28)) != 0)) {
                    mal_uint64 xrc = mal_xgetbv(0);
                    if ((xrc & 0x06) == 0x06) {
                        return MAL_TRUE;
                    } else {
                        return MAL_FALSE;
                    }
                } else {
                    return MAL_FALSE;
                }
            #endif
        #endif
    #else
        return MAL_FALSE;       // AVX is only supported on x86 and x64 architectures.
    #endif
#else
    return MAL_FALSE;           // No compiler support.
#endif
}
#endif

static MAL_INLINE mal_bool32 mal_has_avx2()
{
#if defined(MAL_SUPPORT_AVX2)
    #if (defined(MAL_X64) || defined(MAL_X86)) && !defined(MAL_NO_AVX2)
        #if defined(_AVX2_) || defined(__AVX2__)
            return MAL_TRUE;    // If the compiler is allowed to freely generate AVX2 code we can assume support.
        #else
            // AVX2 requires both CPU and OS support.
            #if defined(MAL_NO_CPUID) || defined(MAL_NO_XGETBV)
                return MAL_FALSE;
            #else
                int info1[4];
                int info7[4];
                mal_cpuid(info1, 1);
                mal_cpuid(info7, 7);
                if (((info1[2] & (1 << 27)) != 0) && ((info7[1] & (1 << 5)) != 0)) {
                    mal_uint64 xrc = mal_xgetbv(0);
                    if ((xrc & 0x06) == 0x06) {
                        return MAL_TRUE;
                    } else {
                        return MAL_FALSE;
                    }
                } else {
                    return MAL_FALSE;
                }
            #endif
        #endif
    #else
        return MAL_FALSE;       // AVX2 is only supported on x86 and x64 architectures.
    #endif
#else
    return MAL_FALSE;           // No compiler support.
#endif
}

static MAL_INLINE mal_bool32 mal_has_avx512f()
{
#if defined(MAL_SUPPORT_AVX512)
    #if (defined(MAL_X64) || defined(MAL_X86)) && !defined(MAL_NO_AVX512)
        #if defined(__AVX512F__)
            return MAL_TRUE;    // If the compiler is allowed to freely generate AVX-512F code we can assume support.
        #else
            // AVX-512 requires both CPU and OS support.
            #if defined(MAL_NO_CPUID) || defined(MAL_NO_XGETBV)
                return MAL_FALSE;
            #else
                int info1[4];
                int info7[4];
                mal_cpuid(info1, 1);
                mal_cpuid(info7, 7);
                if (((info1[2] & (1 << 27)) != 0) && ((info7[1] & (1 << 16)) != 0)) {
                    mal_uint64 xrc = mal_xgetbv(0);
                    if ((xrc & 0xE6) == 0xE6) {
                        return MAL_TRUE;
                    } else {
                        return MAL_FALSE;
                    }
                } else {
                    return MAL_FALSE;
                }
            #endif
        #endif
    #else
        return MAL_FALSE;       // AVX-512F is only supported on x86 and x64 architectures.
    #endif
#else
    return MAL_FALSE;           // No compiler support.
#endif
}

static MAL_INLINE mal_bool32 mal_has_neon()
{
#if defined(MAL_SUPPORT_NEON)
    #if defined(MAL_ARM) && !defined(MAL_NO_NEON)
        #if (defined(__ARM_NEON) || defined(__aarch64__) || defined(_M_ARM64))
            return MAL_TRUE;    // If the compiler is allowed to freely generate NEON code we can assume support.
        #else
            // TODO: Runtime check.
            return MAL_FALSE;
        #endif
    #else
        return MAL_FALSE;       // NEON is only supported on ARM architectures.
    #endif
#else
    return MAL_FALSE;           // No compiler support.
#endif
}


static MAL_INLINE mal_bool32 mal_is_little_endian()
{
#if defined(MAL_X86) || defined(MAL_X64)
    return MAL_TRUE;
#else
    int n = 1;
    return (*(char*)&n) == 1;
#endif
}

static MAL_INLINE mal_bool32 mal_is_big_endian()
{
    return !mal_is_little_endian();
}


#ifndef MAL_COINIT_VALUE
#define MAL_COINIT_VALUE    0   /* 0 = COINIT_MULTITHREADED*/
#endif



#ifndef MAL_PI
#define MAL_PI      3.14159265358979323846264f
#endif
#ifndef MAL_PI_D
#define MAL_PI_D    3.14159265358979323846264
#endif
#ifndef MAL_TAU
#define MAL_TAU     6.28318530717958647693f
#endif
#ifndef MAL_TAU_D
#define MAL_TAU_D   6.28318530717958647693
#endif


// The default format when mal_format_unknown (0) is requested when initializing a device.
#ifndef MAL_DEFAULT_FORMAT
#define MAL_DEFAULT_FORMAT                                  mal_format_f32
#endif

// The default channel count to use when 0 is used when initializing a device.
#ifndef MAL_DEFAULT_CHANNELS
#define MAL_DEFAULT_CHANNELS                                2
#endif

// The default sample rate to use when 0 is used when initializing a device.
#ifndef MAL_DEFAULT_SAMPLE_RATE
#define MAL_DEFAULT_SAMPLE_RATE                             48000
#endif

// Default periods when none is specified in mal_device_init(). More periods means more work on the CPU.
#ifndef MAL_DEFAULT_PERIODS
#define MAL_DEFAULT_PERIODS                                 2
#endif

// The base buffer size in milliseconds for low latency mode.
#ifndef MAL_BASE_BUFFER_SIZE_IN_MILLISECONDS_LOW_LATENCY
#define MAL_BASE_BUFFER_SIZE_IN_MILLISECONDS_LOW_LATENCY    25
#endif

// The base buffer size in milliseconds for conservative mode.
#ifndef MAL_BASE_BUFFER_SIZE_IN_MILLISECONDS_CONSERVATIVE
#define MAL_BASE_BUFFER_SIZE_IN_MILLISECONDS_CONSERVATIVE   150
#endif


// Standard sample rates, in order of priority.
mal_uint32 g_malStandardSampleRatePriorities[] = {
    MAL_SAMPLE_RATE_48000,  // Most common
    MAL_SAMPLE_RATE_44100,

    MAL_SAMPLE_RATE_32000,  // Lows
    MAL_SAMPLE_RATE_24000,
    MAL_SAMPLE_RATE_22050,

    MAL_SAMPLE_RATE_88200,  // Highs
    MAL_SAMPLE_RATE_96000,
    MAL_SAMPLE_RATE_176400,
    MAL_SAMPLE_RATE_192000,

    MAL_SAMPLE_RATE_16000,  // Extreme lows
    MAL_SAMPLE_RATE_11025,
    MAL_SAMPLE_RATE_8000,

    MAL_SAMPLE_RATE_352800, // Extreme highs
    MAL_SAMPLE_RATE_384000
};

mal_format g_malFormatPriorities[] = {
    mal_format_s16,         // Most common
    mal_format_f32,
    
    //mal_format_s24_32,    // Clean alignment
    mal_format_s32,
    
    mal_format_s24,         // Unclean alignment
    
    mal_format_u8           // Low quality
};



///////////////////////////////////////////////////////////////////////////////
//
// Standard Library Stuff
//
///////////////////////////////////////////////////////////////////////////////
#ifndef MAL_MALLOC
#ifdef MAL_WIN32
#define MAL_MALLOC(sz) HeapAlloc(GetProcessHeap(), 0, (sz))
#else
#define MAL_MALLOC(sz) malloc((sz))
#endif
#endif

#ifndef MAL_REALLOC
#ifdef MAL_WIN32
#define MAL_REALLOC(p, sz) (((sz) > 0) ? ((p) ? HeapReAlloc(GetProcessHeap(), 0, (p), (sz)) : HeapAlloc(GetProcessHeap(), 0, (sz))) : ((VOID*)(size_t)(HeapFree(GetProcessHeap(), 0, (p)) & 0)))
#else
#define MAL_REALLOC(p, sz) realloc((p), (sz))
#endif
#endif

#ifndef MAL_FREE
#ifdef MAL_WIN32
#define MAL_FREE(p) HeapFree(GetProcessHeap(), 0, (p))
#else
#define MAL_FREE(p) free((p))
#endif
#endif

#ifndef MAL_ZERO_MEMORY
#ifdef MAL_WIN32
#define MAL_ZERO_MEMORY(p, sz) ZeroMemory((p), (sz))
#else
#define MAL_ZERO_MEMORY(p, sz) memset((p), 0, (sz))
#endif
#endif

#ifndef MAL_COPY_MEMORY
#ifdef MAL_WIN32
#define MAL_COPY_MEMORY(dst, src, sz) CopyMemory((dst), (src), (sz))
#else
#define MAL_COPY_MEMORY(dst, src, sz) memcpy((dst), (src), (sz))
#endif
#endif

#ifndef MAL_ASSERT
#ifdef MAL_WIN32
#define MAL_ASSERT(condition) assert(condition)
#else
#define MAL_ASSERT(condition) assert(condition)
#endif
#endif

#define mal_zero_memory MAL_ZERO_MEMORY
#define mal_copy_memory MAL_COPY_MEMORY
#define mal_assert      MAL_ASSERT

#define mal_zero_object(p)          mal_zero_memory((p), sizeof(*(p)))
#define mal_countof(x)              (sizeof(x) / sizeof(x[0]))
#define mal_max(x, y)               (((x) > (y)) ? (x) : (y))
#define mal_min(x, y)               (((x) < (y)) ? (x) : (y))
#define mal_clamp(x, lo, hi)        (mal_max(lo, mal_min(x, hi)))
#define mal_offset_ptr(p, offset)   (((mal_uint8*)(p)) + (offset))

#define mal_buffer_frame_capacity(buffer, channels, format) (sizeof(buffer) / mal_get_bytes_per_sample(format) / (channels))


// Return Values:
//   0:  Success
//   22: EINVAL
//   34: ERANGE
//
// Not using symbolic constants for errors because I want to avoid #including errno.h
int mal_strcpy_s(char* dst, size_t dstSizeInBytes, const char* src)
{
    if (dst == 0) {
        return 22;
    }
    if (dstSizeInBytes == 0) {
        return 34;
    }
    if (src == 0) {
        dst[0] = '\0';
        return 22;
    }

    size_t i;
    for (i = 0; i < dstSizeInBytes && src[i] != '\0'; ++i) {
        dst[i] = src[i];
    }

    if (i < dstSizeInBytes) {
        dst[i] = '\0';
        return 0;
    }

    dst[0] = '\0';
    return 34;
}

int mal_strncpy_s(char* dst, size_t dstSizeInBytes, const char* src, size_t count)
{
    if (dst == 0) {
        return 22;
    }
    if (dstSizeInBytes == 0) {
        return 34;
    }
    if (src == 0) {
        dst[0] = '\0';
        return 22;
    }

    size_t maxcount = count;
    if (count == ((size_t)-1) || count >= dstSizeInBytes) {        // -1 = _TRUNCATE
        maxcount = dstSizeInBytes - 1;
    }

    size_t i;
    for (i = 0; i < maxcount && src[i] != '\0'; ++i) {
        dst[i] = src[i];
    }

    if (src[i] == '\0' || i == count || count == ((size_t)-1)) {
        dst[i] = '\0';
        return 0;
    }

    dst[0] = '\0';
    return 34;
}

int mal_strcat_s(char* dst, size_t dstSizeInBytes, const char* src)
{
    if (dst == 0) {
        return 22;
    }
    if (dstSizeInBytes == 0) {
        return 34;
    }
    if (src == 0) {
        dst[0] = '\0';
        return 22;
    }

    char* dstorig = dst;

    while (dstSizeInBytes > 0 && dst[0] != '\0') {
        dst += 1;
        dstSizeInBytes -= 1;
    }

    if (dstSizeInBytes == 0) {
        return 22;  // Unterminated.
    }


    while (dstSizeInBytes > 0 && src[0] != '\0') {
        *dst++ = *src++;
        dstSizeInBytes -= 1;
    }

    if (dstSizeInBytes > 0) {
        dst[0] = '\0';
    } else {
        dstorig[0] = '\0';
        return 34;
    }

    return 0;
}

int mal_itoa_s(int value, char* dst, size_t dstSizeInBytes, int radix)
{
    if (dst == NULL || dstSizeInBytes == 0) {
        return 22;
    }
    if (radix < 2 || radix > 36) {
        dst[0] = '\0';
        return 22;
    }

    int sign = (value < 0 && radix == 10) ? -1 : 1;     // The negative sign is only used when the base is 10.

    unsigned int valueU;
    if (value < 0) {
        valueU = -value;
    } else {
        valueU = value;
    }

    char* dstEnd = dst;
    do
    {
        int remainder = valueU % radix;
        if (remainder > 9) {
            *dstEnd = (char)((remainder - 10) + 'a');
        } else {
            *dstEnd = (char)(remainder + '0');
        }

        dstEnd += 1;
        dstSizeInBytes -= 1;
        valueU /= radix;
    } while (dstSizeInBytes > 0 && valueU > 0);

    if (dstSizeInBytes == 0) {
        dst[0] = '\0';
        return 22;  // Ran out of room in the output buffer.
    }

    if (sign < 0) {
        *dstEnd++ = '-';
        dstSizeInBytes -= 1;
    }

    if (dstSizeInBytes == 0) {
        dst[0] = '\0';
        return 22;  // Ran out of room in the output buffer.
    }

    *dstEnd = '\0';


    // At this point the string will be reversed.
    dstEnd -= 1;
    while (dst < dstEnd) {
        char temp = *dst;
        *dst = *dstEnd;
        *dstEnd = temp;

        dst += 1;
        dstEnd -= 1;
    }

    return 0;
}

int mal_strcmp(const char* str1, const char* str2)
{
    if (str1 == str2) return  0;

    // These checks differ from the standard implementation. It's not important, but I prefer
    // it just for sanity.
    if (str1 == NULL) return -1;
    if (str2 == NULL) return  1;

    for (;;) {
        if (str1[0] == '\0') {
            break;
        }
        if (str1[0] != str2[0]) {
            break;
        }

        str1 += 1;
        str2 += 1;
    }

    return ((unsigned char*)str1)[0] - ((unsigned char*)str2)[0];
}


// Thanks to good old Bit Twiddling Hacks for this one: http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
static MAL_INLINE unsigned int mal_next_power_of_2(unsigned int x)
{
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x++;

    return x;
}

static MAL_INLINE unsigned int mal_prev_power_of_2(unsigned int x)
{
    return mal_next_power_of_2(x) >> 1;
}

static MAL_INLINE unsigned int mal_round_to_power_of_2(unsigned int x)
{
    unsigned int prev = mal_prev_power_of_2(x);
    unsigned int next = mal_next_power_of_2(x);
    if ((next - x) > (x - prev)) {
        return prev;
    } else {
        return next;
    }
}

static MAL_INLINE unsigned int mal_count_set_bits(unsigned int x)
{
    unsigned int count = 0;
    while (x != 0) {
        if (x & 1) {
            count += 1;
        }
        
        x = x >> 1;
    }
    
    return count;
}



// Clamps an f32 sample to -1..1
static MAL_INLINE float mal_clip_f32(float x)
{
    if (x < -1) return -1;
    if (x > +1) return +1;
    return x;
}

static MAL_INLINE float mal_mix_f32(float x, float y, float a)
{
    return x*(1-a) + y*a;
}
static MAL_INLINE float mal_mix_f32_fast(float x, float y, float a)
{
    float r0 = (y - x);
    float r1 = r0*a;
    return x + r1;
    //return x + (y - x)*a;
}

#if defined(MAL_SUPPORT_SSE2)
static MAL_INLINE __m128 mal_mix_f32_fast__sse2(__m128 x, __m128 y, __m128 a)
{
    return _mm_add_ps(x, _mm_mul_ps(_mm_sub_ps(y, x), a));
}
#endif
#if defined(MAL_SUPPORT_AVX2)
static MAL_INLINE __m256 mal_mix_f32_fast__avx2(__m256 x, __m256 y, __m256 a)
{
    return _mm256_add_ps(x, _mm256_mul_ps(_mm256_sub_ps(y, x), a));
}
#endif
#if defined(MAL_SUPPORT_AVX512)
static MAL_INLINE __m512 mal_mix_f32_fast__avx512(__m512 x, __m512 y, __m512 a)
{
    return _mm512_add_ps(x, _mm512_mul_ps(_mm512_sub_ps(y, x), a));
}
#endif
#if defined(MAL_SUPPORT_NEON)
static MAL_INLINE float32x4_t mal_mix_f32_fast__neon(float32x4_t x, float32x4_t y, float32x4_t a)
{
    return vaddq_f32(x, vmulq_f32(vsubq_f32(y, x), a));
}
#endif


static MAL_INLINE double mal_mix_f64(double x, double y, double a)
{
    return x*(1-a) + y*a;
}
static MAL_INLINE double mal_mix_f64_fast(double x, double y, double a)
{
    return x + (y - x)*a;
}

static MAL_INLINE float mal_scale_to_range_f32(float x, float lo, float hi)
{
    return lo + x*(hi-lo);
}



// Random Number Generation
//
// mini_al uses the LCG random number generation algorithm. This is good enough for audio.
//
// Note that mini_al's LCG implementation uses global state which is _not_ thread-local. When this is called across
// multiple threads, results will be unpredictable. However, it won't crash and results will still be random enough
// for mini_al's purposes.
#define MAL_LCG_M   4294967296
#define MAL_LCG_A   1103515245
#define MAL_LCG_C   12345
static mal_int32 g_malLCG;

void mal_seed(mal_int32 seed)
{
    g_malLCG = seed;
}

mal_int32 mal_rand_s32()
{
    mal_int32 lcg = g_malLCG;
    mal_int32 r = (MAL_LCG_A * lcg + MAL_LCG_C) % MAL_LCG_M;
    g_malLCG = r;
    return r;
}

double mal_rand_f64()
{
    return (mal_rand_s32() + 0x80000000) / (double)0x7FFFFFFF;
}

float mal_rand_f32()
{
    return (float)mal_rand_f64();
}

static MAL_INLINE float mal_rand_range_f32(float lo, float hi)
{
    return mal_scale_to_range_f32(mal_rand_f32(), lo, hi);
}

static MAL_INLINE mal_int32 mal_rand_range_s32(mal_int32 lo, mal_int32 hi)
{
    double x = mal_rand_f64();
    return lo + (mal_int32)(x*(hi-lo));
}


static MAL_INLINE float mal_dither_f32_rectangle(float ditherMin, float ditherMax)
{
    return mal_rand_range_f32(ditherMin, ditherMax);
}

static MAL_INLINE float mal_dither_f32_triangle(float ditherMin, float ditherMax)
{
    float a = mal_rand_range_f32(ditherMin, 0);
    float b = mal_rand_range_f32(0, ditherMax);
    return a + b;
}

static MAL_INLINE float mal_dither_f32(mal_dither_mode ditherMode, float ditherMin, float ditherMax)
{
    if (ditherMode == mal_dither_mode_rectangle) {
        return mal_dither_f32_rectangle(ditherMin, ditherMax);
    }
    if (ditherMode == mal_dither_mode_triangle) {
        return mal_dither_f32_triangle(ditherMin, ditherMax);
    }

    return 0;
}

static MAL_INLINE mal_int32 mal_dither_s32(mal_dither_mode ditherMode, mal_int32 ditherMin, mal_int32 ditherMax)
{
    if (ditherMode == mal_dither_mode_rectangle) {
        mal_int32 a = mal_rand_range_s32(ditherMin, ditherMax);
        return a;
    }
    if (ditherMode == mal_dither_mode_triangle) {
        mal_int32 a = mal_rand_range_s32(ditherMin, 0);
        mal_int32 b = mal_rand_range_s32(0, ditherMax);
        return a + b;
    }

    return 0;
}


// Splits a buffer into parts of equal length and of the given alignment. The returned size of the split buffers will be a
// multiple of the alignment. The alignment must be a power of 2.
void mal_split_buffer(void* pBuffer, size_t bufferSize, size_t splitCount, size_t alignment, void** ppBuffersOut, size_t* pSplitSizeOut)
{
    if (pSplitSizeOut) {
        *pSplitSizeOut = 0;
    }

    if (pBuffer == NULL || bufferSize == 0 || splitCount == 0) {
        return;
    }

    if (alignment == 0) {
        alignment = 1;
    }

    mal_uintptr pBufferUnaligned = (mal_uintptr)pBuffer;
    mal_uintptr pBufferAligned = (pBufferUnaligned + (alignment-1)) & ~(alignment-1);
    size_t unalignedBytes = (size_t)(pBufferAligned - pBufferUnaligned);

    size_t splitSize = 0;
    if (bufferSize >= unalignedBytes) {
        splitSize = (bufferSize - unalignedBytes) / splitCount;
        splitSize = splitSize & ~(alignment-1);
    }

    if (ppBuffersOut != NULL) {
        for (size_t i = 0; i < splitCount; ++i) {
            ppBuffersOut[i] = (mal_uint8*)(pBufferAligned + (splitSize*i));
        }
    }

    if (pSplitSizeOut) {
        *pSplitSizeOut = splitSize;
    }
}


///////////////////////////////////////////////////////////////////////////////
//
// Atomics
//
///////////////////////////////////////////////////////////////////////////////
#if defined(_WIN32) && !defined(__GNUC__)
#define mal_memory_barrier()            MemoryBarrier()
#define mal_atomic_exchange_32(a, b)    InterlockedExchange((LONG*)a, (LONG)b)
#define mal_atomic_exchange_64(a, b)    InterlockedExchange64((LONGLONG*)a, (LONGLONG)b)
#define mal_atomic_increment_32(a)      InterlockedIncrement((LONG*)a)
#define mal_atomic_decrement_32(a)      InterlockedDecrement((LONG*)a)
#else
#define mal_memory_barrier()            __sync_synchronize()
#define mal_atomic_exchange_32(a, b)    (void)__sync_lock_test_and_set(a, b); __sync_synchronize()
#define mal_atomic_exchange_64(a, b)    (void)__sync_lock_test_and_set(a, b); __sync_synchronize()
#define mal_atomic_increment_32(a)      __sync_add_and_fetch(a, 1)
#define mal_atomic_decrement_32(a)      __sync_sub_and_fetch(a, 1)
#endif

#ifdef MAL_64BIT
#define mal_atomic_exchange_ptr mal_atomic_exchange_64
#endif
#ifdef MAL_32BIT
#define mal_atomic_exchange_ptr mal_atomic_exchange_32
#endif


mal_uint32 mal_get_standard_sample_rate_priority_index(mal_uint32 sampleRate)   // Lower = higher priority
{
    for (mal_uint32 i = 0; i < mal_countof(g_malStandardSampleRatePriorities); ++i) {
        if (g_malStandardSampleRatePriorities[i] == sampleRate) {
            return i;
        }
    }

    return (mal_uint32)-1;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// DEVICE I/O
// ==========
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef MAL_NO_DEVICE_IO
// Unfortunately using runtime linking for pthreads causes problems. This has occurred for me when testing on FreeBSD. When
// using runtime linking, deadlocks can occur (for me it happens when loading data from fread()). It turns out that doing
// compile-time linking fixes this. I'm not sure why this happens, but the safest way I can think of to fix this is to simply
// disable runtime linking by default. To enable runtime linking, #define this before the implementation of this file. I am
// not officially supporting this, but I'm leaving it here in case it's useful for somebody, somewhere.
//#define MAL_USE_RUNTIME_LINKING_FOR_PTHREAD

// Disable run-time linking on certain backends.
#ifndef MAL_NO_RUNTIME_LINKING
    #if defined(MAL_ANDROID) || defined(MAL_EMSCRIPTEN)
        #define MAL_NO_RUNTIME_LINKING
    #endif
#endif

// Check if we have the necessary development packages for each backend at the top so we can use this to determine whether or not
// certain unused functions and variables can be excluded from the build to avoid warnings.
#ifdef MAL_ENABLE_WASAPI
    #define MAL_HAS_WASAPI      // Every compiler should support WASAPI
#endif
#ifdef MAL_ENABLE_DSOUND
    #define MAL_HAS_DSOUND      // Every compiler should support DirectSound.
#endif
#ifdef MAL_ENABLE_WINMM
    #define MAL_HAS_WINMM       // Every compiler I'm aware of supports WinMM.
#endif
#ifdef MAL_ENABLE_ALSA
    #define MAL_HAS_ALSA
    #ifdef MAL_NO_RUNTIME_LINKING
        #ifdef __has_include
            #if !__has_include(<alsa/asoundlib.h>)
                #undef MAL_HAS_ALSA
            #endif
        #endif
    #endif
#endif
#ifdef MAL_ENABLE_PULSEAUDIO
    #define MAL_HAS_PULSEAUDIO  // Development packages are unnecessary for PulseAudio.
    #ifdef MAL_NO_RUNTIME_LINKING
        #ifdef __has_include
            #if !__has_include(<pulse/pulseaudio.h>)
                #undef MAL_HAS_PULSEAUDIO
            #endif
        #endif
    #endif
#endif
#ifdef MAL_ENABLE_JACK
    #define MAL_HAS_JACK
    #ifdef MAL_NO_RUNTIME_LINKING
        #ifdef __has_include
            #if !__has_include(<jack/jack.h>)
                #undef MAL_HAS_JACK
            #endif
        #endif
    #endif
#endif
#ifdef MAL_ENABLE_COREAUDIO
    #define MAL_HAS_COREAUDIO
#endif
#ifdef MAL_ENABLE_SNDIO
    #define MAL_HAS_SNDIO
#endif
#ifdef MAL_ENABLE_AUDIO4
    #define MAL_HAS_AUDIO4      // When enabled, always assume audio(4) is available.
#endif
#ifdef MAL_ENABLE_OSS
    #define MAL_HAS_OSS         // OSS is the only supported backend for Unix and BSD, so it must be present else this library is useless.
#endif
#ifdef MAL_ENABLE_OPENSL
    #define MAL_HAS_OPENSL      // OpenSL is the only supported backend for Android. It must be present.
#endif
#ifdef MAL_ENABLE_OPENAL
    #define MAL_HAS_OPENAL
    #ifdef MAL_NO_RUNTIME_LINKING
        #ifdef __has_include
            #if !__has_include(<AL/al.h>)
                #undef MAL_HAS_OPENAL
            #endif
        #endif
    #endif
#endif
#ifdef MAL_ENABLE_SDL
    #define MAL_HAS_SDL

    // SDL headers are necessary if using compile-time linking.
    #ifdef MAL_NO_RUNTIME_LINKING
        #ifdef __has_include
            #ifdef MAL_EMSCRIPTEN
                #if !__has_include(<SDL/SDL_audio.h>)
                    #undef MAL_HAS_SDL
                #endif
            #else
                #if !__has_include(<SDL2/SDL_audio.h>)
                    #undef MAL_HAS_SDL
                #endif
            #endif
        #endif
    #endif
#endif
#ifdef MAL_ENABLE_NULL
    #define MAL_HAS_NULL    // Everything supports the null backend.
#endif

const mal_backend g_malDefaultBackends[] = {
    mal_backend_wasapi,
    mal_backend_dsound,
    mal_backend_winmm,
    mal_backend_coreaudio,
    mal_backend_sndio,
    mal_backend_audio4,
    mal_backend_oss,
    mal_backend_pulseaudio,
    mal_backend_alsa,
    mal_backend_jack,
    mal_backend_opensl,
    mal_backend_openal,
    mal_backend_sdl,
    mal_backend_null
};

const char* mal_get_backend_name(mal_backend backend)
{
    switch (backend)
    {
        case mal_backend_null:       return "Null";
        case mal_backend_wasapi:     return "WASAPI";
        case mal_backend_dsound:     return "DirectSound";
        case mal_backend_winmm:      return "WinMM";
        case mal_backend_alsa:       return "ALSA";
        case mal_backend_pulseaudio: return "PulseAudio";
        case mal_backend_jack:       return "JACK";
        case mal_backend_coreaudio:  return "Core Audio";
        case mal_backend_sndio:      return "sndio";
        case mal_backend_audio4:     return "audio(4)";
        case mal_backend_oss:        return "OSS";
        case mal_backend_opensl:     return "OpenSL|ES";
        case mal_backend_openal:     return "OpenAL";
        case mal_backend_sdl:        return "SDL";
        default:                     return "Unknown";
    }
}



#ifdef MAL_WIN32
    #define MAL_THREADCALL WINAPI
    typedef unsigned long mal_thread_result;
#else
    #define MAL_THREADCALL
    typedef void* mal_thread_result;
#endif
typedef mal_thread_result (MAL_THREADCALL * mal_thread_entry_proc)(void* pData);

#ifdef MAL_WIN32
typedef HRESULT (WINAPI * MAL_PFN_CoInitializeEx)(LPVOID pvReserved, DWORD  dwCoInit);
typedef void    (WINAPI * MAL_PFN_CoUninitialize)();
typedef HRESULT (WINAPI * MAL_PFN_CoCreateInstance)(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv);
typedef void    (WINAPI * MAL_PFN_CoTaskMemFree)(LPVOID pv);
typedef HRESULT (WINAPI * MAL_PFN_PropVariantClear)(PROPVARIANT *pvar);
typedef int     (WINAPI * MAL_PFN_StringFromGUID2)(const GUID* const rguid, LPOLESTR lpsz, int cchMax);

typedef HWND (WINAPI * MAL_PFN_GetForegroundWindow)();
typedef HWND (WINAPI * MAL_PFN_GetDesktopWindow)();

// Microsoft documents these APIs as returning LSTATUS, but the Win32 API shipping with some compilers do not define it. It's just a LONG.
typedef LONG (WINAPI * MAL_PFN_RegOpenKeyExA)(HKEY hKey, LPCSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult);
typedef LONG (WINAPI * MAL_PFN_RegCloseKey)(HKEY hKey);
typedef LONG (WINAPI * MAL_PFN_RegQueryValueExA)(HKEY hKey, LPCSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
#endif


#define MAL_STATE_UNINITIALIZED     0
#define MAL_STATE_STOPPED           1   // The device's default state after initialization.
#define MAL_STATE_STARTED           2   // The worker thread is in it's main loop waiting for the driver to request or deliver audio data.
#define MAL_STATE_STARTING          3   // Transitioning from a stopped state to started.
#define MAL_STATE_STOPPING          4   // Transitioning from a started state to stopped.

#define MAL_DEFAULT_PLAYBACK_DEVICE_NAME    "Default Playback Device"
#define MAL_DEFAULT_CAPTURE_DEVICE_NAME     "Default Capture Device"


///////////////////////////////////////////////////////////////////////////////
//
// Timing
//
///////////////////////////////////////////////////////////////////////////////
#ifdef MAL_WIN32
LARGE_INTEGER g_mal_TimerFrequency = {{0}};
void mal_timer_init(mal_timer* pTimer)
{
    if (g_mal_TimerFrequency.QuadPart == 0) {
        QueryPerformanceFrequency(&g_mal_TimerFrequency);
    }

    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    pTimer->counter = counter.QuadPart;
}

double mal_timer_get_time_in_seconds(mal_timer* pTimer)
{
    LARGE_INTEGER counter;
    if (!QueryPerformanceCounter(&counter)) {
        return 0;
    }

    return (double)(counter.QuadPart - pTimer->counter) / g_mal_TimerFrequency.QuadPart;
}
#elif defined(MAL_APPLE) && (__MAC_OS_X_VERSION_MIN_REQUIRED < 101200)
mal_uint64 g_mal_TimerFrequency = 0;
void mal_timer_init(mal_timer* pTimer)
{
    mach_timebase_info_data_t baseTime;
    mach_timebase_info(&baseTime);
    g_mal_TimerFrequency = (baseTime.denom * 1e9) / baseTime.numer;

    pTimer->counter = mach_absolute_time();
}

double mal_timer_get_time_in_seconds(mal_timer* pTimer)
{
    mal_uint64 newTimeCounter = mach_absolute_time();
    mal_uint64 oldTimeCounter = pTimer->counter;

    return (newTimeCounter - oldTimeCounter) / g_mal_TimerFrequency;
}
#else
#if defined(CLOCK_MONOTONIC)
    #define MAL_CLOCK_ID CLOCK_MONOTONIC
#else
    #define MAL_CLOCK_ID CLOCK_REALTIME
#endif

void mal_timer_init(mal_timer* pTimer)
{
    struct timespec newTime;
    clock_gettime(MAL_CLOCK_ID, &newTime);

    pTimer->counter = (newTime.tv_sec * 1000000000) + newTime.tv_nsec;
}

double mal_timer_get_time_in_seconds(mal_timer* pTimer)
{
    struct timespec newTime;
    clock_gettime(MAL_CLOCK_ID, &newTime);

    mal_uint64 newTimeCounter = (newTime.tv_sec * 1000000000) + newTime.tv_nsec;
    mal_uint64 oldTimeCounter = pTimer->counter;

    return (newTimeCounter - oldTimeCounter) / 1000000000.0;
}
#endif


///////////////////////////////////////////////////////////////////////////////
//
// Dynamic Linking
//
///////////////////////////////////////////////////////////////////////////////
mal_handle mal_dlopen(const char* filename)
{
#ifdef _WIN32
#ifdef MAL_WIN32_DESKTOP
    return (mal_handle)LoadLibraryA(filename);
#else
    // *sigh* It appears there is no ANSI version of LoadPackagedLibrary()...
    WCHAR filenameW[4096];
    if (MultiByteToWideChar(CP_UTF8, 0, filename, -1, filenameW, sizeof(filenameW)) == 0) {
        return NULL;
    }

    return (mal_handle)LoadPackagedLibrary(filenameW, 0);
#endif
#else
    return (mal_handle)dlopen(filename, RTLD_NOW);
#endif
}

void mal_dlclose(mal_handle handle)
{
#ifdef _WIN32
    FreeLibrary((HMODULE)handle);
#else
    dlclose((void*)handle);
#endif
}

mal_proc mal_dlsym(mal_handle handle, const char* symbol)
{
#ifdef _WIN32
    return (mal_proc)GetProcAddress((HMODULE)handle, symbol);
#else
    return (mal_proc)dlsym((void*)handle, symbol);
#endif
}


///////////////////////////////////////////////////////////////////////////////
//
// Threading
//
///////////////////////////////////////////////////////////////////////////////
#ifdef MAL_WIN32
int mal_thread_priority_to_win32(mal_thread_priority priority)
{
    switch (priority) {
        case mal_thread_priority_idle:     return THREAD_PRIORITY_IDLE;
        case mal_thread_priority_lowest:   return THREAD_PRIORITY_LOWEST;
        case mal_thread_priority_low:      return THREAD_PRIORITY_BELOW_NORMAL;
        case mal_thread_priority_normal:   return THREAD_PRIORITY_NORMAL;
        case mal_thread_priority_high:     return THREAD_PRIORITY_ABOVE_NORMAL;
        case mal_thread_priority_highest:  return THREAD_PRIORITY_HIGHEST;
        case mal_thread_priority_realtime: return THREAD_PRIORITY_TIME_CRITICAL;
        default: return mal_thread_priority_normal;
    }
}

mal_result mal_thread_create__win32(mal_context* pContext, mal_thread* pThread, mal_thread_entry_proc entryProc, void* pData)
{
    pThread->win32.hThread = CreateThread(NULL, 0, entryProc, pData, 0, NULL);
    if (pThread->win32.hThread == NULL) {
        return MAL_FAILED_TO_CREATE_THREAD;
    }

    SetThreadPriority((HANDLE)pThread->win32.hThread, mal_thread_priority_to_win32(pContext->config.threadPriority));

    return MAL_SUCCESS;
}

void mal_thread_wait__win32(mal_thread* pThread)
{
    WaitForSingleObject(pThread->win32.hThread, INFINITE);
}

void mal_sleep__win32(mal_uint32 milliseconds)
{
    Sleep((DWORD)milliseconds);
}


mal_result mal_mutex_init__win32(mal_context* pContext, mal_mutex* pMutex)
{
    (void)pContext;

    pMutex->win32.hMutex = CreateEventA(NULL, FALSE, TRUE, NULL);
    if (pMutex->win32.hMutex == NULL) {
        return MAL_FAILED_TO_CREATE_MUTEX;
    }

    return MAL_SUCCESS;
}

void mal_mutex_uninit__win32(mal_mutex* pMutex)
{
    CloseHandle(pMutex->win32.hMutex);
}

void mal_mutex_lock__win32(mal_mutex* pMutex)
{
    WaitForSingleObject(pMutex->win32.hMutex, INFINITE);
}

void mal_mutex_unlock__win32(mal_mutex* pMutex)
{
    SetEvent(pMutex->win32.hMutex);
}


mal_result mal_event_init__win32(mal_context* pContext, mal_event* pEvent)
{
    (void)pContext;

    pEvent->win32.hEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
    if (pEvent->win32.hEvent == NULL) {
        return MAL_FAILED_TO_CREATE_EVENT;
    }

    return MAL_SUCCESS;
}

void mal_event_uninit__win32(mal_event* pEvent)
{
    CloseHandle(pEvent->win32.hEvent);
}

mal_bool32 mal_event_wait__win32(mal_event* pEvent)
{
    return WaitForSingleObject(pEvent->win32.hEvent, INFINITE) == WAIT_OBJECT_0;
}

mal_bool32 mal_event_signal__win32(mal_event* pEvent)
{
    return SetEvent(pEvent->win32.hEvent);
}
#endif


#ifdef MAL_POSIX
#include <sched.h>

typedef int (* mal_pthread_create_proc)(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg);
typedef int (* mal_pthread_join_proc)(pthread_t thread, void **retval);
typedef int (* mal_pthread_mutex_init_proc)(pthread_mutex_t *__mutex, const pthread_mutexattr_t *__mutexattr);
typedef int (* mal_pthread_mutex_destroy_proc)(pthread_mutex_t *__mutex);
typedef int (* mal_pthread_mutex_lock_proc)(pthread_mutex_t *__mutex);
typedef int (* mal_pthread_mutex_unlock_proc)(pthread_mutex_t *__mutex);
typedef int (* mal_pthread_cond_init_proc)(pthread_cond_t *__restrict __cond, const pthread_condattr_t *__restrict __cond_attr);
typedef int (* mal_pthread_cond_destroy_proc)(pthread_cond_t *__cond);
typedef int (* mal_pthread_cond_signal_proc)(pthread_cond_t *__cond);
typedef int (* mal_pthread_cond_wait_proc)(pthread_cond_t *__restrict __cond, pthread_mutex_t *__restrict __mutex);
typedef int (* mal_pthread_attr_init_proc)(pthread_attr_t *attr);
typedef int (* mal_pthread_attr_destroy_proc)(pthread_attr_t *attr);
typedef int (* mal_pthread_attr_setschedpolicy_proc)(pthread_attr_t *attr, int policy);
typedef int (* mal_pthread_attr_getschedparam_proc)(const pthread_attr_t *attr, struct sched_param *param);
typedef int (* mal_pthread_attr_setschedparam_proc)(pthread_attr_t *attr, const struct sched_param *param);

mal_bool32 mal_thread_create__posix(mal_context* pContext, mal_thread* pThread, mal_thread_entry_proc entryProc, void* pData)
{
    pthread_attr_t* pAttr = NULL;

#if !defined(__EMSCRIPTEN__)
    // Try setting the thread priority. It's not critical if anything fails here.
    pthread_attr_t attr;
    if (((mal_pthread_attr_init_proc)pContext->posix.pthread_attr_init)(&attr) == 0) {
        int scheduler = -1;
        if (pContext->config.threadPriority == mal_thread_priority_idle) {
#ifdef SCHED_IDLE
            if (((mal_pthread_attr_setschedpolicy_proc)pContext->posix.pthread_attr_setschedpolicy)(&attr, SCHED_IDLE) == 0) {
                scheduler = SCHED_IDLE;
            }
#endif
        } else if (pContext->config.threadPriority == mal_thread_priority_realtime) {
#ifdef SCHED_FIFO
            if (((mal_pthread_attr_setschedpolicy_proc)pContext->posix.pthread_attr_setschedpolicy)(&attr, SCHED_FIFO) == 0) {
                scheduler = SCHED_FIFO;
            }
#endif
#ifdef MAL_LINUX
        } else {
            scheduler = sched_getscheduler(0);
#endif
        }

        if (scheduler != -1) {
            int priorityMin = sched_get_priority_min(scheduler);
            int priorityMax = sched_get_priority_max(scheduler);
            int priorityStep = (priorityMax - priorityMin) / 7;  // 7 = number of priorities supported by mini_al.

            struct sched_param sched;
            if (((mal_pthread_attr_getschedparam_proc)pContext->posix.pthread_attr_getschedparam)(&attr, &sched) == 0) {
                if (pContext->config.threadPriority == mal_thread_priority_idle) {
                    sched.sched_priority = priorityMin;
                } else if (pContext->config.threadPriority == mal_thread_priority_realtime) {
                    sched.sched_priority = priorityMax;
                } else {
                    sched.sched_priority += ((int)pContext->config.threadPriority + 5) * priorityStep;  // +5 because the lowest priority is -5.
                    if (sched.sched_priority < priorityMin) {
                        sched.sched_priority = priorityMin;
                    }
                    if (sched.sched_priority > priorityMax) {
                        sched.sched_priority = priorityMax;
                    }
                }

                if (((mal_pthread_attr_setschedparam_proc)pContext->posix.pthread_attr_setschedparam)(&attr, &sched) == 0) {
                    pAttr = &attr;
                }
            }
        }

        ((mal_pthread_attr_destroy_proc)pContext->posix.pthread_attr_destroy)(&attr);
    }
#endif

    int result = ((mal_pthread_create_proc)pContext->posix.pthread_create)(&pThread->posix.thread, pAttr, entryProc, pData);
    if (result != 0) {
        return MAL_FAILED_TO_CREATE_THREAD;
    }

    return MAL_SUCCESS;
}

void mal_thread_wait__posix(mal_thread* pThread)
{
    ((mal_pthread_join_proc)pThread->pContext->posix.pthread_join)(pThread->posix.thread, NULL);
}

void mal_sleep__posix(mal_uint32 milliseconds)
{
    usleep(milliseconds * 1000);    // <-- usleep is in microseconds.
}


mal_result mal_mutex_init__posix(mal_context* pContext, mal_mutex* pMutex)
{
    int result = ((mal_pthread_mutex_init_proc)pContext->posix.pthread_mutex_init)(&pMutex->posix.mutex, NULL);
    if (result != 0) {
        return MAL_FAILED_TO_CREATE_MUTEX;
    }

    return MAL_SUCCESS;
}

void mal_mutex_uninit__posix(mal_mutex* pMutex)
{
    ((mal_pthread_mutex_destroy_proc)pMutex->pContext->posix.pthread_mutex_destroy)(&pMutex->posix.mutex);
}

void mal_mutex_lock__posix(mal_mutex* pMutex)
{
    ((mal_pthread_mutex_lock_proc)pMutex->pContext->posix.pthread_mutex_lock)(&pMutex->posix.mutex);
}

void mal_mutex_unlock__posix(mal_mutex* pMutex)
{
    ((mal_pthread_mutex_unlock_proc)pMutex->pContext->posix.pthread_mutex_unlock)(&pMutex->posix.mutex);
}


mal_result mal_event_init__posix(mal_context* pContext, mal_event* pEvent)
{
    if (((mal_pthread_mutex_init_proc)pContext->posix.pthread_mutex_init)(&pEvent->posix.mutex, NULL) != 0) {
        return MAL_FAILED_TO_CREATE_MUTEX;
    }

    if (((mal_pthread_cond_init_proc)pContext->posix.pthread_cond_init)(&pEvent->posix.condition, NULL) != 0) {
        return MAL_FAILED_TO_CREATE_EVENT;
    }

    pEvent->posix.value = 0;
    return MAL_SUCCESS;
}

void mal_event_uninit__posix(mal_event* pEvent)
{
    ((mal_pthread_cond_destroy_proc)pEvent->pContext->posix.pthread_cond_destroy)(&pEvent->posix.condition);
    ((mal_pthread_mutex_destroy_proc)pEvent->pContext->posix.pthread_mutex_destroy)(&pEvent->posix.mutex);
}

mal_bool32 mal_event_wait__posix(mal_event* pEvent)
{
    ((mal_pthread_mutex_lock_proc)pEvent->pContext->posix.pthread_mutex_lock)(&pEvent->posix.mutex);
    {
        while (pEvent->posix.value == 0) {
            ((mal_pthread_cond_wait_proc)pEvent->pContext->posix.pthread_cond_wait)(&pEvent->posix.condition, &pEvent->posix.mutex);
        }

        pEvent->posix.value = 0;  // Auto-reset.
    }
    ((mal_pthread_mutex_unlock_proc)pEvent->pContext->posix.pthread_mutex_unlock)(&pEvent->posix.mutex);

    return MAL_TRUE;
}

mal_bool32 mal_event_signal__posix(mal_event* pEvent)
{
    ((mal_pthread_mutex_lock_proc)pEvent->pContext->posix.pthread_mutex_lock)(&pEvent->posix.mutex);
    {
        pEvent->posix.value = 1;
        ((mal_pthread_cond_signal_proc)pEvent->pContext->posix.pthread_cond_signal)(&pEvent->posix.condition);
    }
    ((mal_pthread_mutex_unlock_proc)pEvent->pContext->posix.pthread_mutex_unlock)(&pEvent->posix.mutex);

    return MAL_TRUE;
}
#endif

mal_result mal_thread_create(mal_context* pContext, mal_thread* pThread, mal_thread_entry_proc entryProc, void* pData)
{
    if (pContext == NULL || pThread == NULL || entryProc == NULL) return MAL_FALSE;

    pThread->pContext = pContext;

#ifdef MAL_WIN32
    return mal_thread_create__win32(pContext, pThread, entryProc, pData);
#endif
#ifdef MAL_POSIX
    return mal_thread_create__posix(pContext, pThread, entryProc, pData);
#endif
}

void mal_thread_wait(mal_thread* pThread)
{
    if (pThread == NULL) return;

#ifdef MAL_WIN32
    mal_thread_wait__win32(pThread);
#endif
#ifdef MAL_POSIX
    mal_thread_wait__posix(pThread);
#endif
}

void mal_sleep(mal_uint32 milliseconds)
{
#ifdef MAL_WIN32
    mal_sleep__win32(milliseconds);
#endif
#ifdef MAL_POSIX
    mal_sleep__posix(milliseconds);
#endif
}


mal_result mal_mutex_init(mal_context* pContext, mal_mutex* pMutex)
{
    if (pContext == NULL || pMutex == NULL) {
        return MAL_INVALID_ARGS;
    }

    pMutex->pContext = pContext;

#ifdef MAL_WIN32
    return mal_mutex_init__win32(pContext, pMutex);
#endif
#ifdef MAL_POSIX
    return mal_mutex_init__posix(pContext, pMutex);
#endif
}

void mal_mutex_uninit(mal_mutex* pMutex)
{
    if (pMutex == NULL || pMutex->pContext == NULL) return;

#ifdef MAL_WIN32
    mal_mutex_uninit__win32(pMutex);
#endif
#ifdef MAL_POSIX
    mal_mutex_uninit__posix(pMutex);
#endif
}

void mal_mutex_lock(mal_mutex* pMutex)
{
    if (pMutex == NULL || pMutex->pContext == NULL) return;

#ifdef MAL_WIN32
    mal_mutex_lock__win32(pMutex);
#endif
#ifdef MAL_POSIX
    mal_mutex_lock__posix(pMutex);
#endif
}

void mal_mutex_unlock(mal_mutex* pMutex)
{
    if (pMutex == NULL || pMutex->pContext == NULL) return;

#ifdef MAL_WIN32
    mal_mutex_unlock__win32(pMutex);
#endif
#ifdef MAL_POSIX
    mal_mutex_unlock__posix(pMutex);
#endif
}


mal_result mal_event_init(mal_context* pContext, mal_event* pEvent)
{
    if (pContext == NULL || pEvent == NULL) return MAL_FALSE;

    pEvent->pContext = pContext;

#ifdef MAL_WIN32
    return mal_event_init__win32(pContext, pEvent);
#endif
#ifdef MAL_POSIX
    return mal_event_init__posix(pContext, pEvent);
#endif
}

void mal_event_uninit(mal_event* pEvent)
{
    if (pEvent == NULL || pEvent->pContext == NULL) return;

#ifdef MAL_WIN32
    mal_event_uninit__win32(pEvent);
#endif
#ifdef MAL_POSIX
    mal_event_uninit__posix(pEvent);
#endif
}

mal_bool32 mal_event_wait(mal_event* pEvent)
{
    if (pEvent == NULL || pEvent->pContext == NULL) return MAL_FALSE;

#ifdef MAL_WIN32
    return mal_event_wait__win32(pEvent);
#endif
#ifdef MAL_POSIX
    return mal_event_wait__posix(pEvent);
#endif
}

mal_bool32 mal_event_signal(mal_event* pEvent)
{
    if (pEvent == NULL || pEvent->pContext == NULL) return MAL_FALSE;

#ifdef MAL_WIN32
    return mal_event_signal__win32(pEvent);
#endif
#ifdef MAL_POSIX
    return mal_event_signal__posix(pEvent);
#endif
}


mal_uint32 mal_get_best_sample_rate_within_range(mal_uint32 sampleRateMin, mal_uint32 sampleRateMax)
{
    // Normalize the range in case we were given something stupid.
    if (sampleRateMin < MAL_MIN_SAMPLE_RATE) {
        sampleRateMin = MAL_MIN_SAMPLE_RATE;
    }
    if (sampleRateMax > MAL_MAX_SAMPLE_RATE) {
        sampleRateMax = MAL_MAX_SAMPLE_RATE;
    }
    if (sampleRateMin > sampleRateMax) {
        sampleRateMin = sampleRateMax;
    }

    if (sampleRateMin == sampleRateMax) {
        return sampleRateMax;
    } else {
        for (size_t iStandardRate = 0; iStandardRate < mal_countof(g_malStandardSampleRatePriorities); ++iStandardRate) {
            mal_uint32 standardRate = g_malStandardSampleRatePriorities[iStandardRate];
            if (standardRate >= sampleRateMin && standardRate <= sampleRateMax) {
                return standardRate;
            }
        }
    }

    // Should never get here.
    mal_assert(MAL_FALSE);
    return 0;
}

mal_uint32 mal_get_closest_standard_sample_rate(mal_uint32 sampleRateIn)
{
    mal_uint32 closestRate = 0;
    mal_uint32 closestDiff = 0xFFFFFFFF;

    for (size_t iStandardRate = 0; iStandardRate < mal_countof(g_malStandardSampleRatePriorities); ++iStandardRate) {
        mal_uint32 standardRate = g_malStandardSampleRatePriorities[iStandardRate];

        mal_uint32 diff;
        if (sampleRateIn > standardRate) {
            diff = sampleRateIn - standardRate;
        } else {
            diff = standardRate - sampleRateIn;
        }

        if (diff == 0) {
            return standardRate;    // The input sample rate is a standard rate.
        }

        if (closestDiff > diff) {
            closestDiff = diff;
            closestRate = standardRate;
        }
    }

    return closestRate;
}


mal_uint32 mal_scale_buffer_size(mal_uint32 baseBufferSize, float scale)
{
    return mal_max(1, (mal_uint32)(baseBufferSize*scale));
}

mal_uint32 mal_calculate_buffer_size_in_milliseconds_from_frames(mal_uint32 bufferSizeInFrames, mal_uint32 sampleRate)
{
    return bufferSizeInFrames / (sampleRate/1000);
}

mal_uint32 mal_calculate_buffer_size_in_frames_from_milliseconds(mal_uint32 bufferSizeInMilliseconds, mal_uint32 sampleRate)
{
    return bufferSizeInMilliseconds * (sampleRate/1000); 
}

mal_uint32 mal_get_default_buffer_size_in_milliseconds(mal_performance_profile performanceProfile)
{
    if (performanceProfile == mal_performance_profile_low_latency) {
        return MAL_BASE_BUFFER_SIZE_IN_MILLISECONDS_LOW_LATENCY;
    } else {
        return MAL_BASE_BUFFER_SIZE_IN_MILLISECONDS_CONSERVATIVE;
    }
}

mal_uint32 mal_get_default_buffer_size_in_frames(mal_performance_profile performanceProfile, mal_uint32 sampleRate)
{
    mal_uint32 bufferSizeInMilliseconds = mal_get_default_buffer_size_in_milliseconds(performanceProfile);
    if (bufferSizeInMilliseconds == 0) {
        bufferSizeInMilliseconds = 1;
    }

    mal_uint32 sampleRateMS = (sampleRate/1000);
    if (sampleRateMS == 0) {
        sampleRateMS = 1;
    }

    return bufferSizeInMilliseconds * sampleRateMS;
}


const char* mal_log_level_to_string(mal_uint32 logLevel)
{
    switch (logLevel)
    {
        case MAL_LOG_LEVEL_VERBOSE: return "";
        case MAL_LOG_LEVEL_INFO:    return "INFO";
        case MAL_LOG_LEVEL_WARNING: return "WARNING";
        case MAL_LOG_LEVEL_ERROR:   return "ERROR";
        default:                    return "ERROR";
    }
}

// Posts a log message.
void mal_log(mal_context* pContext, mal_device* pDevice, mal_uint32 logLevel, const char* message)
{
    if (pContext == NULL) return;
    
#if defined(MAL_LOG_LEVEL)
    if (logLevel <= MAL_LOG_LEVEL) {
    #if defined(MAL_DEBUG_OUTPUT)
        if (logLevel <= MAL_LOG_LEVEL) {
            printf("%s: %s", mal_log_level_to_string(logLevel), message);
        }
    #endif
    
        mal_log_proc onLog = pContext->config.onLog;
        if (onLog) {
            onLog(pContext, pDevice, message);
        }
    }
#endif
}

// Posts an error. Throw a breakpoint in here if you're needing to debug. The return value is always "resultCode".
mal_result mal_context_post_error(mal_context* pContext, mal_device* pDevice, mal_uint32 logLevel, const char* message, mal_result resultCode)
{
    // Derive the context from the device if necessary.
    if (pContext == NULL) {
        if (pDevice != NULL) {
            pContext = pDevice->pContext;
        }
    }

    mal_log(pContext, pDevice, logLevel, message);
    return resultCode;
}

mal_result mal_post_error(mal_device* pDevice, mal_uint32 logLevel, const char* message, mal_result resultCode)
{
    return mal_context_post_error(NULL, pDevice, logLevel, message, resultCode);
}


// The callback for reading from the client -> DSP -> device.
mal_uint32 mal_device__on_read_from_client(mal_dsp* pDSP, mal_uint32 frameCount, void* pFramesOut, void* pUserData)
{
    (void)pDSP;

    mal_device* pDevice = (mal_device*)pUserData;
    mal_assert(pDevice != NULL);

    mal_send_proc onSend = pDevice->onSend;
    if (onSend) {
        return onSend(pDevice, frameCount, pFramesOut);
    }

    return 0;
}

// The callback for reading from the device -> DSP -> client.
mal_uint32 mal_device__on_read_from_device(mal_dsp* pDSP, mal_uint32 frameCount, void* pFramesOut, void* pUserData)
{
    (void)pDSP;

    mal_device* pDevice = (mal_device*)pUserData;
    mal_assert(pDevice != NULL);

    if (pDevice->_dspFrameCount == 0) {
        return 0;   // Nothing left.
    }

    mal_uint32 framesToRead = frameCount;
    if (framesToRead > pDevice->_dspFrameCount) {
        framesToRead = pDevice->_dspFrameCount;
    }

    mal_uint32 bytesToRead = framesToRead * pDevice->internalChannels * mal_get_bytes_per_sample(pDevice->internalFormat);
    mal_copy_memory(pFramesOut, pDevice->_dspFrames, bytesToRead);
    pDevice->_dspFrameCount -= framesToRead;
    pDevice->_dspFrames += bytesToRead;

    return framesToRead;
}

// A helper function for reading sample data from the client. Returns the number of samples read from the client. Remaining samples
// are filled with silence.
static MAL_INLINE mal_uint32 mal_device__read_frames_from_client(mal_device* pDevice, mal_uint32 frameCount, void* pSamples)
{
    mal_assert(pDevice != NULL);
    mal_assert(frameCount > 0);
    mal_assert(pSamples != NULL);

    mal_uint32 framesRead     = (mal_uint32)mal_dsp_read(&pDevice->dsp, frameCount, pSamples, pDevice->dsp.pUserData);
    mal_uint32 samplesRead    = framesRead * pDevice->internalChannels;
    mal_uint32 sampleSize     = mal_get_bytes_per_sample(pDevice->internalFormat);
    mal_uint32 consumedBytes  = samplesRead*sampleSize;
    mal_uint32 remainingBytes = ((frameCount * pDevice->internalChannels) - samplesRead)*sampleSize;
    mal_zero_memory((mal_uint8*)pSamples + consumedBytes, remainingBytes);

    return samplesRead;
}

// A helper for sending sample data to the client.
static MAL_INLINE void mal_device__send_frames_to_client(mal_device* pDevice, mal_uint32 frameCount, const void* pSamples)
{
    mal_assert(pDevice != NULL);
    mal_assert(frameCount > 0);
    mal_assert(pSamples != NULL);

    mal_recv_proc onRecv = pDevice->onRecv;
    if (onRecv) {
        pDevice->_dspFrameCount = frameCount;
        pDevice->_dspFrames = (const mal_uint8*)pSamples;

        mal_uint8 chunkBuffer[4096];
        mal_uint32 chunkFrameCount = sizeof(chunkBuffer) / mal_get_bytes_per_frame(pDevice->format, pDevice->channels);

        for (;;) {
            mal_uint32 framesJustRead = (mal_uint32)mal_dsp_read(&pDevice->dsp, chunkFrameCount, chunkBuffer, pDevice->dsp.pUserData);
            if (framesJustRead == 0) {
                break;
            }

            onRecv(pDevice, framesJustRead, chunkBuffer);

            if (framesJustRead < chunkFrameCount) {
                break;
            }
        }
    }
}

// A helper for changing the state of the device.
static MAL_INLINE void mal_device__set_state(mal_device* pDevice, mal_uint32 newState)
{
    mal_atomic_exchange_32(&pDevice->state, newState);
}

// A helper for getting the state of the device.
static MAL_INLINE mal_uint32 mal_device__get_state(mal_device* pDevice)
{
    return pDevice->state;
}


#ifdef MAL_WIN32
    GUID MAL_GUID_KSDATAFORMAT_SUBTYPE_PCM        = {0x00000001, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}};
    GUID MAL_GUID_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT = {0x00000003, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}};
    //GUID MAL_GUID_KSDATAFORMAT_SUBTYPE_ALAW       = {0x00000006, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}};
    //GUID MAL_GUID_KSDATAFORMAT_SUBTYPE_MULAW      = {0x00000007, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}};
#endif


mal_bool32 mal_context__device_id_equal(mal_context* pContext, const mal_device_id* pID0, const mal_device_id* pID1)
{
    mal_assert(pContext != NULL);

    if (pID0 == pID1) return MAL_TRUE;

    if ((pID0 == NULL && pID1 != NULL) ||
        (pID0 != NULL && pID1 == NULL)) {
        return MAL_FALSE;
    }

    if (pContext->onDeviceIDEqual) {
        return pContext->onDeviceIDEqual(pContext, pID0, pID1);
    }

    return MAL_FALSE;
}


typedef struct
{
    mal_device_type deviceType;
    const mal_device_id* pDeviceID;
    char* pName;
    size_t nameBufferSize;
    mal_bool32 foundDevice;
} mal_context__try_get_device_name_by_id__enum_callback_data;

mal_bool32 mal_context__try_get_device_name_by_id__enum_callback(mal_context* pContext, mal_device_type deviceType, const mal_device_info* pDeviceInfo, void* pUserData)
{
    mal_context__try_get_device_name_by_id__enum_callback_data* pData = (mal_context__try_get_device_name_by_id__enum_callback_data*)pUserData;
    mal_assert(pData != NULL);

    if (pData->deviceType == deviceType) {
        if (pContext->onDeviceIDEqual(pContext, pData->pDeviceID, &pDeviceInfo->id)) {
            mal_strncpy_s(pData->pName, pData->nameBufferSize, pDeviceInfo->name, (size_t)-1);
            pData->foundDevice = MAL_TRUE;
        }
    }

    return !pData->foundDevice;
}

// Generic function for retrieving the name of a device by it's ID.
//
// This function simply enumerates every device and then retrieves the name of the first device that has the same ID.
mal_result mal_context__try_get_device_name_by_id(mal_context* pContext, mal_device_type type, const mal_device_id* pDeviceID, char* pName, size_t nameBufferSize)
{
    mal_assert(pContext != NULL);
    mal_assert(pName != NULL);

    if (pDeviceID == NULL) {
        return MAL_NO_DEVICE;
    }

    mal_context__try_get_device_name_by_id__enum_callback_data data;
    data.deviceType = type;
    data.pDeviceID = pDeviceID;
    data.pName = pName;
    data.nameBufferSize = nameBufferSize;
    data.foundDevice = MAL_FALSE;
    mal_result result = mal_context_enumerate_devices(pContext, mal_context__try_get_device_name_by_id__enum_callback, &data);
    if (result != MAL_SUCCESS) {
        return result;
    }

    if (!data.foundDevice) {
        return MAL_NO_DEVICE;
    } else {
        return MAL_SUCCESS;
    }
}


mal_uint32 mal_get_format_priority_index(mal_format format) // Lower = better.
{
    for (mal_uint32 i = 0; i < mal_countof(g_malFormatPriorities); ++i) {
        if (g_malFormatPriorities[i] == format) {
            return i;
        }
    }

    // Getting here means the format could not be found or is equal to mal_format_unknown.
    return (mal_uint32)-1;
}

void mal_device__post_init_setup(mal_device* pDevice);


///////////////////////////////////////////////////////////////////////////////
//
// WIN32 COMMON
//
///////////////////////////////////////////////////////////////////////////////
#if defined(MAL_WIN32)
#include "objbase.h"
#if defined(MAL_WIN32_DESKTOP)
    #define mal_CoInitializeEx(pContext, pvReserved, dwCoInit)                          ((MAL_PFN_CoInitializeEx)pContext->win32.CoInitializeEx)(pvReserved, dwCoInit)
    #define mal_CoUninitialize(pContext)                                                ((MAL_PFN_CoUninitialize)pContext->win32.CoUninitialize)()
    #define mal_CoCreateInstance(pContext, rclsid, pUnkOuter, dwClsContext, riid, ppv)  ((MAL_PFN_CoCreateInstance)pContext->win32.CoCreateInstance)(rclsid, pUnkOuter, dwClsContext, riid, ppv)
    #define mal_CoTaskMemFree(pContext, pv)                                             ((MAL_PFN_CoTaskMemFree)pContext->win32.CoTaskMemFree)(pv)
    #define mal_PropVariantClear(pContext, pvar)                                        ((MAL_PFN_PropVariantClear)pContext->win32.PropVariantClear)(pvar)
#else
    #define mal_CoInitializeEx(pContext, pvReserved, dwCoInit)                          CoInitializeEx(pvReserved, dwCoInit)
    #define mal_CoUninitialize(pContext)                                                CoUninitialize()
    #define mal_CoCreateInstance(pContext, rclsid, pUnkOuter, dwClsContext, riid, ppv)  CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv)
    #define mal_CoTaskMemFree(pContext, pv)                                             CoTaskMemFree(pv)
    #define mal_PropVariantClear(pContext, pvar)                                        PropVariantClear(pvar)
#endif

// There's a few common headers for Win32 backends which include here for simplicity. Note that we should never
// include any files that do not come standard with modern compilers, and we may need to manually define a few
// symbols.
#include <mmreg.h>
#include <mmsystem.h>

#if !defined(MAXULONG_PTR)
typedef size_t DWORD_PTR;
#endif

#if !defined(WAVE_FORMAT_44M08)
#define WAVE_FORMAT_44M08 0x00000100
#define WAVE_FORMAT_44S08 0x00000200
#define WAVE_FORMAT_44M16 0x00000400
#define WAVE_FORMAT_44S16 0x00000800
#define WAVE_FORMAT_48M08 0x00001000
#define WAVE_FORMAT_48S08 0x00002000
#define WAVE_FORMAT_48M16 0x00004000
#define WAVE_FORMAT_48S16 0x00008000
#define WAVE_FORMAT_96M08 0x00010000
#define WAVE_FORMAT_96S08 0x00020000
#define WAVE_FORMAT_96M16 0x00040000
#define WAVE_FORMAT_96S16 0x00080000
#endif

#ifndef SPEAKER_FRONT_LEFT
#define SPEAKER_FRONT_LEFT            0x1
#define SPEAKER_FRONT_RIGHT           0x2
#define SPEAKER_FRONT_CENTER          0x4
#define SPEAKER_LOW_FREQUENCY         0x8
#define SPEAKER_BACK_LEFT             0x10
#define SPEAKER_BACK_RIGHT            0x20
#define SPEAKER_FRONT_LEFT_OF_CENTER  0x40
#define SPEAKER_FRONT_RIGHT_OF_CENTER 0x80
#define SPEAKER_BACK_CENTER           0x100
#define SPEAKER_SIDE_LEFT             0x200
#define SPEAKER_SIDE_RIGHT            0x400
#define SPEAKER_TOP_CENTER            0x800
#define SPEAKER_TOP_FRONT_LEFT        0x1000
#define SPEAKER_TOP_FRONT_CENTER      0x2000
#define SPEAKER_TOP_FRONT_RIGHT       0x4000
#define SPEAKER_TOP_BACK_LEFT         0x8000
#define SPEAKER_TOP_BACK_CENTER       0x10000
#define SPEAKER_TOP_BACK_RIGHT        0x20000
#endif

// The SDK that comes with old versions of MSVC (VC6, for example) does not appear to define WAVEFORMATEXTENSIBLE. We
// define our own implementation in this case.
#if (defined(_MSC_VER) && !defined(_WAVEFORMATEXTENSIBLE_)) || defined(__DMC__)
typedef struct
{
    WAVEFORMATEX Format;
    union
    {
        WORD wValidBitsPerSample;
        WORD wSamplesPerBlock;
        WORD wReserved;
    } Samples;
    DWORD dwChannelMask;
    GUID SubFormat;
} WAVEFORMATEXTENSIBLE;
#endif

#ifndef WAVE_FORMAT_EXTENSIBLE
#define WAVE_FORMAT_EXTENSIBLE  0xFFFE
#endif

#ifndef WAVE_FORMAT_IEEE_FLOAT
#define WAVE_FORMAT_IEEE_FLOAT  0x0003
#endif

GUID MAL_GUID_NULL = {0x00000000, 0x0000, 0x0000, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

// Converts an individual Win32-style channel identifier (SPEAKER_FRONT_LEFT, etc.) to mini_al.
mal_uint8 mal_channel_id_to_mal__win32(DWORD id)
{
    switch (id)
    {
        case SPEAKER_FRONT_LEFT:            return MAL_CHANNEL_FRONT_LEFT;
        case SPEAKER_FRONT_RIGHT:           return MAL_CHANNEL_FRONT_RIGHT;
        case SPEAKER_FRONT_CENTER:          return MAL_CHANNEL_FRONT_CENTER;
        case SPEAKER_LOW_FREQUENCY:         return MAL_CHANNEL_LFE;
        case SPEAKER_BACK_LEFT:             return MAL_CHANNEL_BACK_LEFT;
        case SPEAKER_BACK_RIGHT:            return MAL_CHANNEL_BACK_RIGHT;
        case SPEAKER_FRONT_LEFT_OF_CENTER:  return MAL_CHANNEL_FRONT_LEFT_CENTER;
        case SPEAKER_FRONT_RIGHT_OF_CENTER: return MAL_CHANNEL_FRONT_RIGHT_CENTER;
        case SPEAKER_BACK_CENTER:           return MAL_CHANNEL_BACK_CENTER;
        case SPEAKER_SIDE_LEFT:             return MAL_CHANNEL_SIDE_LEFT;
        case SPEAKER_SIDE_RIGHT:            return MAL_CHANNEL_SIDE_RIGHT;
        case SPEAKER_TOP_CENTER:            return MAL_CHANNEL_TOP_CENTER;
        case SPEAKER_TOP_FRONT_LEFT:        return MAL_CHANNEL_TOP_FRONT_LEFT;
        case SPEAKER_TOP_FRONT_CENTER:      return MAL_CHANNEL_TOP_FRONT_CENTER;
        case SPEAKER_TOP_FRONT_RIGHT:       return MAL_CHANNEL_TOP_FRONT_RIGHT;
        case SPEAKER_TOP_BACK_LEFT:         return MAL_CHANNEL_TOP_BACK_LEFT;
        case SPEAKER_TOP_BACK_CENTER:       return MAL_CHANNEL_TOP_BACK_CENTER;
        case SPEAKER_TOP_BACK_RIGHT:        return MAL_CHANNEL_TOP_BACK_RIGHT;
        default: return 0;
    }
}

// Converts an individual mini_al channel identifier (MAL_CHANNEL_FRONT_LEFT, etc.) to Win32-style.
DWORD mal_channel_id_to_win32(DWORD id)
{
    switch (id)
    {
        case MAL_CHANNEL_MONO:               return SPEAKER_FRONT_CENTER;
        case MAL_CHANNEL_FRONT_LEFT:         return SPEAKER_FRONT_LEFT;
        case MAL_CHANNEL_FRONT_RIGHT:        return SPEAKER_FRONT_RIGHT;
        case MAL_CHANNEL_FRONT_CENTER:       return SPEAKER_FRONT_CENTER;
        case MAL_CHANNEL_LFE:                return SPEAKER_LOW_FREQUENCY;
        case MAL_CHANNEL_BACK_LEFT:          return SPEAKER_BACK_LEFT;
        case MAL_CHANNEL_BACK_RIGHT:         return SPEAKER_BACK_RIGHT;
        case MAL_CHANNEL_FRONT_LEFT_CENTER:  return SPEAKER_FRONT_LEFT_OF_CENTER;
        case MAL_CHANNEL_FRONT_RIGHT_CENTER: return SPEAKER_FRONT_RIGHT_OF_CENTER;
        case MAL_CHANNEL_BACK_CENTER:        return SPEAKER_BACK_CENTER;
        case MAL_CHANNEL_SIDE_LEFT:          return SPEAKER_SIDE_LEFT;
        case MAL_CHANNEL_SIDE_RIGHT:         return SPEAKER_SIDE_RIGHT;
        case MAL_CHANNEL_TOP_CENTER:         return SPEAKER_TOP_CENTER;
        case MAL_CHANNEL_TOP_FRONT_LEFT:     return SPEAKER_TOP_FRONT_LEFT;
        case MAL_CHANNEL_TOP_FRONT_CENTER:   return SPEAKER_TOP_FRONT_CENTER;
        case MAL_CHANNEL_TOP_FRONT_RIGHT:    return SPEAKER_TOP_FRONT_RIGHT;
        case MAL_CHANNEL_TOP_BACK_LEFT:      return SPEAKER_TOP_BACK_LEFT;
        case MAL_CHANNEL_TOP_BACK_CENTER:    return SPEAKER_TOP_BACK_CENTER;
        case MAL_CHANNEL_TOP_BACK_RIGHT:     return SPEAKER_TOP_BACK_RIGHT;
        default: return 0;
    }
}

// Converts a channel mapping to a Win32-style channel mask.
DWORD mal_channel_map_to_channel_mask__win32(const mal_channel channelMap[MAL_MAX_CHANNELS], mal_uint32 channels)
{
    DWORD dwChannelMask = 0;
    for (mal_uint32 iChannel = 0; iChannel < channels; ++iChannel) {
        dwChannelMask |= mal_channel_id_to_win32(channelMap[iChannel]);
    }

    return dwChannelMask;
}

// Converts a Win32-style channel mask to a mini_al channel map.
void mal_channel_mask_to_channel_map__win32(DWORD dwChannelMask, mal_uint32 channels, mal_channel channelMap[MAL_MAX_CHANNELS])
{
    if (channels == 1 && dwChannelMask == 0) {
        channelMap[0] = MAL_CHANNEL_MONO;
    } else if (channels == 2 && dwChannelMask == 0) {
        channelMap[0] = MAL_CHANNEL_FRONT_LEFT;
        channelMap[1] = MAL_CHANNEL_FRONT_RIGHT;
    } else {
        if (channels == 1 && (dwChannelMask & SPEAKER_FRONT_CENTER) != 0) {
            channelMap[0] = MAL_CHANNEL_MONO;
        } else {
            // Just iterate over each bit.
            mal_uint32 iChannel = 0;
            for (mal_uint32 iBit = 0; iBit < 32; ++iBit) {
                DWORD bitValue = (dwChannelMask & (1UL << iBit));
                if (bitValue != 0) {
                    // The bit is set.
                    channelMap[iChannel] = mal_channel_id_to_mal__win32(bitValue);
                    iChannel += 1;
                }
            }
        }
    }
}

#ifdef __cplusplus
mal_bool32 mal_is_guid_equal(const void* a, const void* b)
{
    return IsEqualGUID(*(const GUID*)a, *(const GUID*)b);
}
#else
#define mal_is_guid_equal(a, b) IsEqualGUID((const GUID*)a, (const GUID*)b)
#endif

mal_format mal_format_from_WAVEFORMATEX(const WAVEFORMATEX* pWF)
{
    mal_assert(pWF != NULL);

    if (pWF->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
        const WAVEFORMATEXTENSIBLE* pWFEX = (const WAVEFORMATEXTENSIBLE*)pWF;
        if (mal_is_guid_equal(&pWFEX->SubFormat, &MAL_GUID_KSDATAFORMAT_SUBTYPE_PCM)) {
            if (pWFEX->Samples.wValidBitsPerSample == 32) {
                return mal_format_s32;
            }
            if (pWFEX->Samples.wValidBitsPerSample == 24) {
                if (pWFEX->Format.wBitsPerSample == 32) {
                    //return mal_format_s24_32;
                }
                if (pWFEX->Format.wBitsPerSample == 24) {
                    return mal_format_s24;
                }
            }
            if (pWFEX->Samples.wValidBitsPerSample == 16) {
                return mal_format_s16;
            }
            if (pWFEX->Samples.wValidBitsPerSample == 8) {
                return mal_format_u8;
            }
        }
        if (mal_is_guid_equal(&pWFEX->SubFormat, &MAL_GUID_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)) {
            if (pWFEX->Samples.wValidBitsPerSample == 32) {
                return mal_format_f32;
            }
            //if (pWFEX->Samples.wValidBitsPerSample == 64) {
            //    return mal_format_f64;
            //}
        }
    } else {
        if (pWF->wFormatTag == WAVE_FORMAT_PCM) {
            if (pWF->wBitsPerSample == 32) {
                return mal_format_s32;
            }
            if (pWF->wBitsPerSample == 24) {
                return mal_format_s24;
            }
            if (pWF->wBitsPerSample == 16) {
                return mal_format_s16;
            }
            if (pWF->wBitsPerSample == 8) {
                return mal_format_u8;
            }
        }
        if (pWF->wFormatTag == WAVE_FORMAT_IEEE_FLOAT) {
            if (pWF->wBitsPerSample == 32) {
                return mal_format_f32;
            }
            if (pWF->wBitsPerSample == 64) {
                //return mal_format_f64;
            }
        }
    }

    return mal_format_unknown;
}
#endif


///////////////////////////////////////////////////////////////////////////////
//
// WASAPI Backend
//
///////////////////////////////////////////////////////////////////////////////
#ifdef MAL_HAS_WASAPI
//#if defined(_MSC_VER)
//    #pragma warning(push)
//    #pragma warning(disable:4091)   // 'typedef ': ignored on left of '' when no variable is declared
//#endif
//#include <audioclient.h>
//#include <mmdeviceapi.h>
//#if defined(_MSC_VER)
//    #pragma warning(pop)
//#endif


// Some compilers don't define VerifyVersionInfoW. Need to write this ourselves.
#if defined(__DMC__)
#define _WIN32_WINNT_VISTA      0x0600
#define VER_MINORVERSION        0x01
#define VER_MAJORVERSION        0x02
#define VER_SERVICEPACKMAJOR    0x20
#define VER_GREATER_EQUAL       0x03

typedef struct  {
    DWORD dwOSVersionInfoSize;
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    DWORD dwBuildNumber;
    DWORD dwPlatformId;
    WCHAR szCSDVersion[128];
    WORD  wServicePackMajor;
    WORD  wServicePackMinor;
    WORD  wSuiteMask;
    BYTE  wProductType;
    BYTE  wReserved;
} mal_OSVERSIONINFOEXW;

BOOL WINAPI VerifyVersionInfoW(mal_OSVERSIONINFOEXW* lpVersionInfo, DWORD dwTypeMask, DWORDLONG dwlConditionMask);
ULONGLONG WINAPI VerSetConditionMask(ULONGLONG dwlConditionMask, DWORD dwTypeBitMask, BYTE dwConditionMask);
#else
typedef OSVERSIONINFOEXW mal_OSVERSIONINFOEXW;
#endif


#ifndef PROPERTYKEY_DEFINED
#define PROPERTYKEY_DEFINED
typedef struct
{
    GUID fmtid;
    DWORD pid;
} PROPERTYKEY;
#endif

// Some compilers don't define PropVariantInit(). We just do this ourselves since it's just a memset().
static MAL_INLINE void mal_PropVariantInit(PROPVARIANT* pProp)
{
    mal_zero_object(pProp);
}


const PROPERTYKEY MAL_PKEY_Device_FriendlyName             = {{0xA45C254E, 0xDF1C, 0x4EFD, {0x80, 0x20, 0x67, 0xD1, 0x46, 0xA8, 0x50, 0xE0}}, 14};
const PROPERTYKEY MAL_PKEY_AudioEngine_DeviceFormat        = {{0xF19F064D, 0x82C,  0x4E27, {0xBC, 0x73, 0x68, 0x82, 0xA1, 0xBB, 0x8E, 0x4C}},  0};

const IID MAL_IID_IUnknown                                 = {0x00000000, 0x0000, 0x0000, {0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}}; // 00000000-0000-0000-C000-000000000046
const IID MAL_IID_IAgileObject                             = {0x94EA2B94, 0xE9CC, 0x49E0, {0xC0, 0xFF, 0xEE, 0x64, 0xCA, 0x8F, 0x5B, 0x90}}; // 94EA2B94-E9CC-49E0-C0FF-EE64CA8F5B90

const IID MAL_IID_IAudioClient                             = {0x1CB9AD4C, 0xDBFA, 0x4C32, {0xB1, 0x78, 0xC2, 0xF5, 0x68, 0xA7, 0x03, 0xB2}}; // 1CB9AD4C-DBFA-4C32-B178-C2F568A703B2 = __uuidof(IAudioClient)
const IID MAL_IID_IAudioClient2                            = {0x726778CD, 0xF60A, 0x4EDA, {0x82, 0xDE, 0xE4, 0x76, 0x10, 0xCD, 0x78, 0xAA}}; // 726778CD-F60A-4EDA-82DE-E47610CD78AA = __uuidof(IAudioClient2)
const IID MAL_IID_IAudioClient3                            = {0x7ED4EE07, 0x8E67, 0x4CD4, {0x8C, 0x1A, 0x2B, 0x7A, 0x59, 0x87, 0xAD, 0x42}}; // 7ED4EE07-8E67-4CD4-8C1A-2B7A5987AD42 = __uuidof(IAudioClient3)
const IID MAL_IID_IAudioRenderClient                       = {0xF294ACFC, 0x3146, 0x4483, {0xA7, 0xBF, 0xAD, 0xDC, 0xA7, 0xC2, 0x60, 0xE2}}; // F294ACFC-3146-4483-A7BF-ADDCA7C260E2 = __uuidof(IAudioRenderClient)
const IID MAL_IID_IAudioCaptureClient                      = {0xC8ADBD64, 0xE71E, 0x48A0, {0xA4, 0xDE, 0x18, 0x5C, 0x39, 0x5C, 0xD3, 0x17}}; // C8ADBD64-E71E-48A0-A4DE-185C395CD317 = __uuidof(IAudioCaptureClient)
const IID MAL_IID_IMMNotificationClient                    = {0x7991EEC9, 0x7E89, 0x4D85, {0x83, 0x90, 0x6C, 0x70, 0x3C, 0xEC, 0x60, 0xC0}}; // 7991EEC9-7E89-4D85-8390-6C703CEC60C0 = __uuidof(IMMNotificationClient)
#ifndef MAL_WIN32_DESKTOP
const IID MAL_IID_DEVINTERFACE_AUDIO_RENDER                = {0xE6327CAD, 0xDCEC, 0x4949, {0xAE, 0x8A, 0x99, 0x1E, 0x97, 0x6A, 0x79, 0xD2}}; // E6327CAD-DCEC-4949-AE8A-991E976A79D2
const IID MAL_IID_DEVINTERFACE_AUDIO_CAPTURE               = {0x2EEF81BE, 0x33FA, 0x4800, {0x96, 0x70, 0x1C, 0xD4, 0x74, 0x97, 0x2C, 0x3F}}; // 2EEF81BE-33FA-4800-9670-1CD474972C3F
const IID MAL_IID_IActivateAudioInterfaceCompletionHandler = {0x41D949AB, 0x9862, 0x444A, {0x80, 0xF6, 0xC2, 0x61, 0x33, 0x4D, 0xA5, 0xEB}}; // 41D949AB-9862-444A-80F6-C261334DA5EB
#endif

const IID MAL_CLSID_MMDeviceEnumerator_Instance            = {0xBCDE0395, 0xE52F, 0x467C, {0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E}}; // BCDE0395-E52F-467C-8E3D-C4579291692E = __uuidof(MMDeviceEnumerator)
const IID MAL_IID_IMMDeviceEnumerator_Instance             = {0xA95664D2, 0x9614, 0x4F35, {0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6}}; // A95664D2-9614-4F35-A746-DE8DB63617E6 = __uuidof(IMMDeviceEnumerator)
#ifdef __cplusplus
#define MAL_CLSID_MMDeviceEnumerator MAL_CLSID_MMDeviceEnumerator_Instance
#define MAL_IID_IMMDeviceEnumerator  MAL_IID_IMMDeviceEnumerator_Instance
#else
#define MAL_CLSID_MMDeviceEnumerator &MAL_CLSID_MMDeviceEnumerator_Instance
#define MAL_IID_IMMDeviceEnumerator  &MAL_IID_IMMDeviceEnumerator_Instance
#endif

typedef struct mal_IUnknown                                 mal_IUnknown;
#ifdef MAL_WIN32_DESKTOP
#define MAL_MM_DEVICE_STATE_ACTIVE                          1
#define MAL_MM_DEVICE_STATE_DISABLED                        2
#define MAL_MM_DEVICE_STATE_NOTPRESENT                      4
#define MAL_MM_DEVICE_STATE_UNPLUGGED                       8

typedef struct mal_IMMDeviceEnumerator                      mal_IMMDeviceEnumerator;
typedef struct mal_IMMDeviceCollection                      mal_IMMDeviceCollection;
typedef struct mal_IMMDevice                                mal_IMMDevice;
#else
typedef struct mal_IActivateAudioInterfaceCompletionHandler mal_IActivateAudioInterfaceCompletionHandler;
typedef struct mal_IActivateAudioInterfaceAsyncOperation    mal_IActivateAudioInterfaceAsyncOperation;
#endif
typedef struct mal_IPropertyStore                           mal_IPropertyStore;
typedef struct mal_IAudioClient                             mal_IAudioClient;
typedef struct mal_IAudioClient2                            mal_IAudioClient2;
typedef struct mal_IAudioClient3                            mal_IAudioClient3;
typedef struct mal_IAudioRenderClient                       mal_IAudioRenderClient;
typedef struct mal_IAudioCaptureClient                      mal_IAudioCaptureClient;

typedef mal_int64                                           MAL_REFERENCE_TIME;

#define MAL_AUDCLNT_STREAMFLAGS_CROSSPROCESS                0x00010000
#define MAL_AUDCLNT_STREAMFLAGS_LOOPBACK                    0x00020000
#define MAL_AUDCLNT_STREAMFLAGS_EVENTCALLBACK               0x00040000
#define MAL_AUDCLNT_STREAMFLAGS_NOPERSIST                   0x00080000
#define MAL_AUDCLNT_STREAMFLAGS_RATEADJUST                  0x00100000
#define MAL_AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY         0x08000000
#define MAL_AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM              0x80000000
#define MAL_AUDCLNT_SESSIONFLAGS_EXPIREWHENUNOWNED          0x10000000
#define MAL_AUDCLNT_SESSIONFLAGS_DISPLAY_HIDE               0x20000000
#define MAL_AUDCLNT_SESSIONFLAGS_DISPLAY_HIDEWHENEXPIRED    0x40000000

// We only care about a few error codes.
#define MAL_AUDCLNT_E_INVALID_DEVICE_PERIOD                 (-2004287456)
#define MAL_AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED               (-2004287463)
#define MAL_AUDCLNT_S_BUFFER_EMPTY                          (143196161)
#define MAL_AUDCLNT_E_DEVICE_IN_USE                         (-2004287478)

typedef enum
{
    mal_eRender  = 0,
    mal_eCapture = 1,
    mal_eAll     = 2
} mal_EDataFlow;

typedef enum
{
    mal_eConsole        = 0,
    mal_eMultimedia     = 1,
    mal_eCommunications = 2
} mal_ERole;

typedef enum
{
    MAL_AUDCLNT_SHAREMODE_SHARED,
    MAL_AUDCLNT_SHAREMODE_EXCLUSIVE
} MAL_AUDCLNT_SHAREMODE;

typedef enum
{
    MAL_AudioCategory_Other = 0,    // <-- mini_al is only caring about Other.
} MAL_AUDIO_STREAM_CATEGORY;

typedef struct
{
    UINT32 cbSize;
    BOOL bIsOffload;
    MAL_AUDIO_STREAM_CATEGORY eCategory;
} mal_AudioClientProperties;

// IUnknown
typedef struct
{
    // IUnknown
    HRESULT (STDMETHODCALLTYPE * QueryInterface)(mal_IUnknown* pThis, const IID* const riid, void** ppObject);
    ULONG   (STDMETHODCALLTYPE * AddRef)        (mal_IUnknown* pThis);
    ULONG   (STDMETHODCALLTYPE * Release)       (mal_IUnknown* pThis);
} mal_IUnknownVtbl;
struct mal_IUnknown
{
    mal_IUnknownVtbl* lpVtbl;
};
HRESULT mal_IUnknown_QueryInterface(mal_IUnknown* pThis, const IID* const riid, void** ppObject) { return pThis->lpVtbl->QueryInterface(pThis, riid, ppObject); }
ULONG   mal_IUnknown_AddRef(mal_IUnknown* pThis)                                                 { return pThis->lpVtbl->AddRef(pThis); }
ULONG   mal_IUnknown_Release(mal_IUnknown* pThis)                                                { return pThis->lpVtbl->Release(pThis); }

#ifdef MAL_WIN32_DESKTOP
    // IMMNotificationClient
    typedef struct
    {
        // IUnknown
        HRESULT (STDMETHODCALLTYPE * QueryInterface)(mal_IMMNotificationClient* pThis, const IID* const riid, void** ppObject);
        ULONG   (STDMETHODCALLTYPE * AddRef)        (mal_IMMNotificationClient* pThis);
        ULONG   (STDMETHODCALLTYPE * Release)       (mal_IMMNotificationClient* pThis);

        // IMMNotificationClient
        HRESULT (STDMETHODCALLTYPE * OnDeviceStateChanged)  (mal_IMMNotificationClient* pThis, LPCWSTR pDeviceID, DWORD dwNewState);
        HRESULT (STDMETHODCALLTYPE * OnDeviceAdded)         (mal_IMMNotificationClient* pThis, LPCWSTR pDeviceID);
        HRESULT (STDMETHODCALLTYPE * OnDeviceRemoved)       (mal_IMMNotificationClient* pThis, LPCWSTR pDeviceID);
        HRESULT (STDMETHODCALLTYPE * OnDefaultDeviceChanged)(mal_IMMNotificationClient* pThis, mal_EDataFlow dataFlow, mal_ERole role, LPCWSTR pDefaultDeviceID);
        HRESULT (STDMETHODCALLTYPE * OnPropertyValueChanged)(mal_IMMNotificationClient* pThis, LPCWSTR pDeviceID, const PROPERTYKEY key);
    } mal_IMMNotificationClientVtbl;

    // IMMDeviceEnumerator
    typedef struct
    {
        // IUnknown
        HRESULT (STDMETHODCALLTYPE * QueryInterface)(mal_IMMDeviceEnumerator* pThis, const IID* const riid, void** ppObject);
        ULONG   (STDMETHODCALLTYPE * AddRef)        (mal_IMMDeviceEnumerator* pThis);
        ULONG   (STDMETHODCALLTYPE * Release)       (mal_IMMDeviceEnumerator* pThis);

        // IMMDeviceEnumerator
        HRESULT (STDMETHODCALLTYPE * EnumAudioEndpoints)                    (mal_IMMDeviceEnumerator* pThis, mal_EDataFlow dataFlow, DWORD dwStateMask, mal_IMMDeviceCollection** ppDevices);
        HRESULT (STDMETHODCALLTYPE * GetDefaultAudioEndpoint)               (mal_IMMDeviceEnumerator* pThis, mal_EDataFlow dataFlow, mal_ERole role, mal_IMMDevice** ppEndpoint);
        HRESULT (STDMETHODCALLTYPE * GetDevice)                             (mal_IMMDeviceEnumerator* pThis, LPCWSTR pID, mal_IMMDevice** ppDevice);
        HRESULT (STDMETHODCALLTYPE * RegisterEndpointNotificationCallback)  (mal_IMMDeviceEnumerator* pThis, mal_IMMNotificationClient* pClient);
        HRESULT (STDMETHODCALLTYPE * UnregisterEndpointNotificationCallback)(mal_IMMDeviceEnumerator* pThis, mal_IMMNotificationClient* pClient);
    } mal_IMMDeviceEnumeratorVtbl;
    struct mal_IMMDeviceEnumerator
    {
        mal_IMMDeviceEnumeratorVtbl* lpVtbl;
    };
    HRESULT mal_IMMDeviceEnumerator_QueryInterface(mal_IMMDeviceEnumerator* pThis, const IID* const riid, void** ppObject) { return pThis->lpVtbl->QueryInterface(pThis, riid, ppObject); }
    ULONG   mal_IMMDeviceEnumerator_AddRef(mal_IMMDeviceEnumerator* pThis)                                                 { return pThis->lpVtbl->AddRef(pThis); }
    ULONG   mal_IMMDeviceEnumerator_Release(mal_IMMDeviceEnumerator* pThis)                                                { return pThis->lpVtbl->Release(pThis); }
    HRESULT mal_IMMDeviceEnumerator_EnumAudioEndpoints(mal_IMMDeviceEnumerator* pThis, mal_EDataFlow dataFlow, DWORD dwStateMask, mal_IMMDeviceCollection** ppDevices) { return pThis->lpVtbl->EnumAudioEndpoints(pThis, dataFlow, dwStateMask, ppDevices); }
    HRESULT mal_IMMDeviceEnumerator_GetDefaultAudioEndpoint(mal_IMMDeviceEnumerator* pThis, mal_EDataFlow dataFlow, mal_ERole role, mal_IMMDevice** ppEndpoint) { return pThis->lpVtbl->GetDefaultAudioEndpoint(pThis, dataFlow, role, ppEndpoint); }
    HRESULT mal_IMMDeviceEnumerator_GetDevice(mal_IMMDeviceEnumerator* pThis, LPCWSTR pID, mal_IMMDevice** ppDevice)       { return pThis->lpVtbl->GetDevice(pThis, pID, ppDevice); }
    HRESULT mal_IMMDeviceEnumerator_RegisterEndpointNotificationCallback(mal_IMMDeviceEnumerator* pThis, mal_IMMNotificationClient* pClient) { return pThis->lpVtbl->RegisterEndpointNotificationCallback(pThis, pClient); }
    HRESULT mal_IMMDeviceEnumerator_UnregisterEndpointNotificationCallback(mal_IMMDeviceEnumerator* pThis, mal_IMMNotificationClient* pClient) { return pThis->lpVtbl->UnregisterEndpointNotificationCallback(pThis, pClient); }


    // IMMDeviceCollection
    typedef struct
    {
        // IUnknown
        HRESULT (STDMETHODCALLTYPE * QueryInterface)(mal_IMMDeviceCollection* pThis, const IID* const riid, void** ppObject);
        ULONG   (STDMETHODCALLTYPE * AddRef)        (mal_IMMDeviceCollection* pThis);
        ULONG   (STDMETHODCALLTYPE * Release)       (mal_IMMDeviceCollection* pThis);

        // IMMDeviceCollection
        HRESULT (STDMETHODCALLTYPE * GetCount)(mal_IMMDeviceCollection* pThis, UINT* pDevices);
        HRESULT (STDMETHODCALLTYPE * Item)    (mal_IMMDeviceCollection* pThis, UINT nDevice, mal_IMMDevice** ppDevice);
    } mal_IMMDeviceCollectionVtbl;
    struct mal_IMMDeviceCollection
    {
        mal_IMMDeviceCollectionVtbl* lpVtbl;
    };
    HRESULT mal_IMMDeviceCollection_QueryInterface(mal_IMMDeviceCollection* pThis, const IID* const riid, void** ppObject) { return pThis->lpVtbl->QueryInterface(pThis, riid, ppObject); }
    ULONG   mal_IMMDeviceCollection_AddRef(mal_IMMDeviceCollection* pThis)                                                 { return pThis->lpVtbl->AddRef(pThis); }
    ULONG   mal_IMMDeviceCollection_Release(mal_IMMDeviceCollection* pThis)                                                { return pThis->lpVtbl->Release(pThis); }
    HRESULT mal_IMMDeviceCollection_GetCount(mal_IMMDeviceCollection* pThis, UINT* pDevices)                               { return pThis->lpVtbl->GetCount(pThis, pDevices); }
    HRESULT mal_IMMDeviceCollection_Item(mal_IMMDeviceCollection* pThis, UINT nDevice, mal_IMMDevice** ppDevice)           { return pThis->lpVtbl->Item(pThis, nDevice, ppDevice); }


    // IMMDevice
    typedef struct
    {
        // IUnknown
        HRESULT (STDMETHODCALLTYPE * QueryInterface)(mal_IMMDevice* pThis, const IID* const riid, void** ppObject);
        ULONG   (STDMETHODCALLTYPE * AddRef)        (mal_IMMDevice* pThis);
        ULONG   (STDMETHODCALLTYPE * Release)       (mal_IMMDevice* pThis);

        // IMMDevice
        HRESULT (STDMETHODCALLTYPE * Activate)         (mal_IMMDevice* pThis, const IID* const iid, DWORD dwClsCtx, PROPVARIANT* pActivationParams, void** ppInterface);
        HRESULT (STDMETHODCALLTYPE * OpenPropertyStore)(mal_IMMDevice* pThis, DWORD stgmAccess, mal_IPropertyStore** ppProperties);
        HRESULT (STDMETHODCALLTYPE * GetId)            (mal_IMMDevice* pThis, LPWSTR *pID);
        HRESULT (STDMETHODCALLTYPE * GetState)         (mal_IMMDevice* pThis, DWORD *pState);
    } mal_IMMDeviceVtbl;
    struct mal_IMMDevice
    {
        mal_IMMDeviceVtbl* lpVtbl;
    };
    HRESULT mal_IMMDevice_QueryInterface(mal_IMMDevice* pThis, const IID* const riid, void** ppObject) { return pThis->lpVtbl->QueryInterface(pThis, riid, ppObject); }
    ULONG   mal_IMMDevice_AddRef(mal_IMMDevice* pThis)                                                 { return pThis->lpVtbl->AddRef(pThis); }
    ULONG   mal_IMMDevice_Release(mal_IMMDevice* pThis)                                                { return pThis->lpVtbl->Release(pThis); }
    HRESULT mal_IMMDevice_Activate(mal_IMMDevice* pThis, const IID* const iid, DWORD dwClsCtx, PROPVARIANT* pActivationParams, void** ppInterface) { return pThis->lpVtbl->Activate(pThis, iid, dwClsCtx, pActivationParams, ppInterface); }
    HRESULT mal_IMMDevice_OpenPropertyStore(mal_IMMDevice* pThis, DWORD stgmAccess, mal_IPropertyStore** ppProperties) { return pThis->lpVtbl->OpenPropertyStore(pThis, stgmAccess, ppProperties); }
    HRESULT mal_IMMDevice_GetId(mal_IMMDevice* pThis, LPWSTR *pID)                                     { return pThis->lpVtbl->GetId(pThis, pID); }
    HRESULT mal_IMMDevice_GetState(mal_IMMDevice* pThis, DWORD *pState)                                { return pThis->lpVtbl->GetState(pThis, pState); }
#else
    // IActivateAudioInterfaceAsyncOperation
    typedef struct
    {
        // IUnknown
        HRESULT (STDMETHODCALLTYPE * QueryInterface)(mal_IActivateAudioInterfaceAsyncOperation* pThis, const IID* const riid, void** ppObject);
        ULONG   (STDMETHODCALLTYPE * AddRef)        (mal_IActivateAudioInterfaceAsyncOperation* pThis);
        ULONG   (STDMETHODCALLTYPE * Release)       (mal_IActivateAudioInterfaceAsyncOperation* pThis);

        // IActivateAudioInterfaceAsyncOperation
        HRESULT (STDMETHODCALLTYPE * GetActivateResult)(mal_IActivateAudioInterfaceAsyncOperation* pThis, HRESULT *pActivateResult, mal_IUnknown** ppActivatedInterface);
    } mal_IActivateAudioInterfaceAsyncOperationVtbl;
    struct mal_IActivateAudioInterfaceAsyncOperation
    {
        mal_IActivateAudioInterfaceAsyncOperationVtbl* lpVtbl;
    };
    HRESULT mal_IActivateAudioInterfaceAsyncOperation_QueryInterface(mal_IActivateAudioInterfaceAsyncOperation* pThis, const IID* const riid, void** ppObject) { return pThis->lpVtbl->QueryInterface(pThis, riid, ppObject); }
    ULONG   mal_IActivateAudioInterfaceAsyncOperation_AddRef(mal_IActivateAudioInterfaceAsyncOperation* pThis)                                                 { return pThis->lpVtbl->AddRef(pThis); }
    ULONG   mal_IActivateAudioInterfaceAsyncOperation_Release(mal_IActivateAudioInterfaceAsyncOperation* pThis)                                                { return pThis->lpVtbl->Release(pThis); }
    HRESULT mal_IActivateAudioInterfaceAsyncOperation_GetActivateResult(mal_IActivateAudioInterfaceAsyncOperation* pThis, HRESULT *pActivateResult, mal_IUnknown** ppActivatedInterface) { return pThis->lpVtbl->GetActivateResult(pThis, pActivateResult, ppActivatedInterface); }
#endif

// IPropertyStore
typedef struct
{
    // IUnknown
    HRESULT (STDMETHODCALLTYPE * QueryInterface)(mal_IPropertyStore* pThis, const IID* const riid, void** ppObject);
    ULONG   (STDMETHODCALLTYPE * AddRef)        (mal_IPropertyStore* pThis);
    ULONG   (STDMETHODCALLTYPE * Release)       (mal_IPropertyStore* pThis);

    // IPropertyStore
    HRESULT (STDMETHODCALLTYPE * GetCount)(mal_IPropertyStore* pThis, DWORD* pPropCount);
    HRESULT (STDMETHODCALLTYPE * GetAt)   (mal_IPropertyStore* pThis, DWORD propIndex, PROPERTYKEY* pPropKey);
    HRESULT (STDMETHODCALLTYPE * GetValue)(mal_IPropertyStore* pThis, const PROPERTYKEY* const pKey, PROPVARIANT* pPropVar);
    HRESULT (STDMETHODCALLTYPE * SetValue)(mal_IPropertyStore* pThis, const PROPERTYKEY* const pKey, const PROPVARIANT* const pPropVar);
    HRESULT (STDMETHODCALLTYPE * Commit)  (mal_IPropertyStore* pThis);
} mal_IPropertyStoreVtbl;
struct mal_IPropertyStore
{
    mal_IPropertyStoreVtbl* lpVtbl;
};
HRESULT mal_IPropertyStore_QueryInterface(mal_IPropertyStore* pThis, const IID* const riid, void** ppObject) { return pThis->lpVtbl->QueryInterface(pThis, riid, ppObject); }
ULONG   mal_IPropertyStore_AddRef(mal_IPropertyStore* pThis)                                                 { return pThis->lpVtbl->AddRef(pThis); }
ULONG   mal_IPropertyStore_Release(mal_IPropertyStore* pThis)                                                { return pThis->lpVtbl->Release(pThis); }
HRESULT mal_IPropertyStore_GetCount(mal_IPropertyStore* pThis, DWORD* pPropCount)                            { return pThis->lpVtbl->GetCount(pThis, pPropCount); }
HRESULT mal_IPropertyStore_GetAt(mal_IPropertyStore* pThis, DWORD propIndex, PROPERTYKEY* pPropKey)          { return pThis->lpVtbl->GetAt(pThis, propIndex, pPropKey); }
HRESULT mal_IPropertyStore_GetValue(mal_IPropertyStore* pThis, const PROPERTYKEY* const pKey, PROPVARIANT* pPropVar) { return pThis->lpVtbl->GetValue(pThis, pKey, pPropVar); }
HRESULT mal_IPropertyStore_SetValue(mal_IPropertyStore* pThis, const PROPERTYKEY* const pKey, const PROPVARIANT* const pPropVar) { return pThis->lpVtbl->SetValue(pThis, pKey, pPropVar); }
HRESULT mal_IPropertyStore_Commit(mal_IPropertyStore* pThis)                                                 { return pThis->lpVtbl->Commit(pThis); }


// IAudioClient
typedef struct
{
    // IUnknown
    HRESULT (STDMETHODCALLTYPE * QueryInterface)(mal_IAudioClient* pThis, const IID* const riid, void** ppObject);
    ULONG   (STDMETHODCALLTYPE * AddRef)        (mal_IAudioClient* pThis);
    ULONG   (STDMETHODCALLTYPE * Release)       (mal_IAudioClient* pThis);

    // IAudioClient
    HRESULT (STDMETHODCALLTYPE * Initialize)       (mal_IAudioClient* pThis, MAL_AUDCLNT_SHAREMODE shareMode, DWORD streamFlags, MAL_REFERENCE_TIME bufferDuration, MAL_REFERENCE_TIME periodicity, const WAVEFORMATEX* pFormat, const GUID* pAudioSessionGuid);
    HRESULT (STDMETHODCALLTYPE * GetBufferSize)    (mal_IAudioClient* pThis, mal_uint32* pNumBufferFrames);
    HRESULT (STDMETHODCALLTYPE * GetStreamLatency) (mal_IAudioClient* pThis, MAL_REFERENCE_TIME* pLatency);
    HRESULT (STDMETHODCALLTYPE * GetCurrentPadding)(mal_IAudioClient* pThis, mal_uint32* pNumPaddingFrames);
    HRESULT (STDMETHODCALLTYPE * IsFormatSupported)(mal_IAudioClient* pThis, MAL_AUDCLNT_SHAREMODE shareMode, const WAVEFORMATEX* pFormat, WAVEFORMATEX** ppClosestMatch);
    HRESULT (STDMETHODCALLTYPE * GetMixFormat)     (mal_IAudioClient* pThis, WAVEFORMATEX** ppDeviceFormat);
    HRESULT (STDMETHODCALLTYPE * GetDevicePeriod)  (mal_IAudioClient* pThis, MAL_REFERENCE_TIME* pDefaultDevicePeriod, MAL_REFERENCE_TIME* pMinimumDevicePeriod);
    HRESULT (STDMETHODCALLTYPE * Start)            (mal_IAudioClient* pThis);
    HRESULT (STDMETHODCALLTYPE * Stop)             (mal_IAudioClient* pThis);
    HRESULT (STDMETHODCALLTYPE * Reset)            (mal_IAudioClient* pThis);
    HRESULT (STDMETHODCALLTYPE * SetEventHandle)   (mal_IAudioClient* pThis, HANDLE eventHandle);
    HRESULT (STDMETHODCALLTYPE * GetService)       (mal_IAudioClient* pThis, const IID* const riid, void** pp);
} mal_IAudioClientVtbl;
struct mal_IAudioClient
{
    mal_IAudioClientVtbl* lpVtbl;
};
HRESULT mal_IAudioClient_QueryInterface(mal_IAudioClient* pThis, const IID* const riid, void** ppObject) { return pThis->lpVtbl->QueryInterface(pThis, riid, ppObject); }
ULONG   mal_IAudioClient_AddRef(mal_IAudioClient* pThis)                                                 { return pThis->lpVtbl->AddRef(pThis); }
ULONG   mal_IAudioClient_Release(mal_IAudioClient* pThis)                                                { return pThis->lpVtbl->Release(pThis); }
HRESULT mal_IAudioClient_Initialize(mal_IAudioClient* pThis, MAL_AUDCLNT_SHAREMODE shareMode, DWORD streamFlags, MAL_REFERENCE_TIME bufferDuration, MAL_REFERENCE_TIME periodicity, const WAVEFORMATEX* pFormat, const GUID* pAudioSessionGuid) { return pThis->lpVtbl->Initialize(pThis, shareMode, streamFlags, bufferDuration, periodicity, pFormat, pAudioSessionGuid); }
HRESULT mal_IAudioClient_GetBufferSize(mal_IAudioClient* pThis, mal_uint32* pNumBufferFrames)                { return pThis->lpVtbl->GetBufferSize(pThis, pNumBufferFrames); }
HRESULT mal_IAudioClient_GetStreamLatency(mal_IAudioClient* pThis, MAL_REFERENCE_TIME* pLatency)             { return pThis->lpVtbl->GetStreamLatency(pThis, pLatency); }
HRESULT mal_IAudioClient_GetCurrentPadding(mal_IAudioClient* pThis, mal_uint32* pNumPaddingFrames)           { return pThis->lpVtbl->GetCurrentPadding(pThis, pNumPaddingFrames); }
HRESULT mal_IAudioClient_IsFormatSupported(mal_IAudioClient* pThis, MAL_AUDCLNT_SHAREMODE shareMode, const WAVEFORMATEX* pFormat, WAVEFORMATEX** ppClosestMatch) { return pThis->lpVtbl->IsFormatSupported(pThis, shareMode, pFormat, ppClosestMatch); }
HRESULT mal_IAudioClient_GetMixFormat(mal_IAudioClient* pThis, WAVEFORMATEX** ppDeviceFormat)            { return pThis->lpVtbl->GetMixFormat(pThis, ppDeviceFormat); }
HRESULT mal_IAudioClient_GetDevicePeriod(mal_IAudioClient* pThis, MAL_REFERENCE_TIME* pDefaultDevicePeriod, MAL_REFERENCE_TIME* pMinimumDevicePeriod) { return pThis->lpVtbl->GetDevicePeriod(pThis, pDefaultDevicePeriod, pMinimumDevicePeriod); }
HRESULT mal_IAudioClient_Start(mal_IAudioClient* pThis)                                                  { return pThis->lpVtbl->Start(pThis); }
HRESULT mal_IAudioClient_Stop(mal_IAudioClient* pThis)                                                   { return pThis->lpVtbl->Stop(pThis); }
HRESULT mal_IAudioClient_Reset(mal_IAudioClient* pThis)                                                  { return pThis->lpVtbl->Reset(pThis); }
HRESULT mal_IAudioClient_SetEventHandle(mal_IAudioClient* pThis, HANDLE eventHandle)                     { return pThis->lpVtbl->SetEventHandle(pThis, eventHandle); }
HRESULT mal_IAudioClient_GetService(mal_IAudioClient* pThis, const IID* const riid, void** pp)           { return pThis->lpVtbl->GetService(pThis, riid, pp); }

// IAudioClient2
typedef struct
{
    // IUnknown
    HRESULT (STDMETHODCALLTYPE * QueryInterface)(mal_IAudioClient2* pThis, const IID* const riid, void** ppObject);
    ULONG   (STDMETHODCALLTYPE * AddRef)        (mal_IAudioClient2* pThis);
    ULONG   (STDMETHODCALLTYPE * Release)       (mal_IAudioClient2* pThis);

    // IAudioClient
    HRESULT (STDMETHODCALLTYPE * Initialize)       (mal_IAudioClient2* pThis, MAL_AUDCLNT_SHAREMODE shareMode, DWORD streamFlags, MAL_REFERENCE_TIME bufferDuration, MAL_REFERENCE_TIME periodicity, const WAVEFORMATEX* pFormat, const GUID* pAudioSessionGuid);
    HRESULT (STDMETHODCALLTYPE * GetBufferSize)    (mal_IAudioClient2* pThis, mal_uint32* pNumBufferFrames);
    HRESULT (STDMETHODCALLTYPE * GetStreamLatency) (mal_IAudioClient2* pThis, MAL_REFERENCE_TIME* pLatency);
    HRESULT (STDMETHODCALLTYPE * GetCurrentPadding)(mal_IAudioClient2* pThis, mal_uint32* pNumPaddingFrames);
    HRESULT (STDMETHODCALLTYPE * IsFormatSupported)(mal_IAudioClient2* pThis, MAL_AUDCLNT_SHAREMODE shareMode, const WAVEFORMATEX* pFormat, WAVEFORMATEX** ppClosestMatch);
    HRESULT (STDMETHODCALLTYPE * GetMixFormat)     (mal_IAudioClient2* pThis, WAVEFORMATEX** ppDeviceFormat);
    HRESULT (STDMETHODCALLTYPE * GetDevicePeriod)  (mal_IAudioClient2* pThis, MAL_REFERENCE_TIME* pDefaultDevicePeriod, MAL_REFERENCE_TIME* pMinimumDevicePeriod);
    HRESULT (STDMETHODCALLTYPE * Start)            (mal_IAudioClient2* pThis);
    HRESULT (STDMETHODCALLTYPE * Stop)             (mal_IAudioClient2* pThis);
    HRESULT (STDMETHODCALLTYPE * Reset)            (mal_IAudioClient2* pThis);
    HRESULT (STDMETHODCALLTYPE * SetEventHandle)   (mal_IAudioClient2* pThis, HANDLE eventHandle);
    HRESULT (STDMETHODCALLTYPE * GetService)       (mal_IAudioClient2* pThis, const IID* const riid, void** pp);

    // IAudioClient2
    HRESULT (STDMETHODCALLTYPE * IsOffloadCapable)   (mal_IAudioClient2* pThis, MAL_AUDIO_STREAM_CATEGORY category, BOOL* pOffloadCapable);
    HRESULT (STDMETHODCALLTYPE * SetClientProperties)(mal_IAudioClient2* pThis, const mal_AudioClientProperties* pProperties);
    HRESULT (STDMETHODCALLTYPE * GetBufferSizeLimits)(mal_IAudioClient2* pThis, const WAVEFORMATEX* pFormat, BOOL eventDriven, MAL_REFERENCE_TIME* pMinBufferDuration, MAL_REFERENCE_TIME* pMaxBufferDuration);
} mal_IAudioClient2Vtbl;
struct mal_IAudioClient2
{
    mal_IAudioClient2Vtbl* lpVtbl;
};
HRESULT mal_IAudioClient2_QueryInterface(mal_IAudioClient2* pThis, const IID* const riid, void** ppObject) { return pThis->lpVtbl->QueryInterface(pThis, riid, ppObject); }
ULONG   mal_IAudioClient2_AddRef(mal_IAudioClient2* pThis)                                                 { return pThis->lpVtbl->AddRef(pThis); }
ULONG   mal_IAudioClient2_Release(mal_IAudioClient2* pThis)                                                { return pThis->lpVtbl->Release(pThis); }
HRESULT mal_IAudioClient2_Initialize(mal_IAudioClient2* pThis, MAL_AUDCLNT_SHAREMODE shareMode, DWORD streamFlags, MAL_REFERENCE_TIME bufferDuration, MAL_REFERENCE_TIME periodicity, const WAVEFORMATEX* pFormat, const GUID* pAudioSessionGuid) { return pThis->lpVtbl->Initialize(pThis, shareMode, streamFlags, bufferDuration, periodicity, pFormat, pAudioSessionGuid); }
HRESULT mal_IAudioClient2_GetBufferSize(mal_IAudioClient2* pThis, mal_uint32* pNumBufferFrames)                { return pThis->lpVtbl->GetBufferSize(pThis, pNumBufferFrames); }
HRESULT mal_IAudioClient2_GetStreamLatency(mal_IAudioClient2* pThis, MAL_REFERENCE_TIME* pLatency)             { return pThis->lpVtbl->GetStreamLatency(pThis, pLatency); }
HRESULT mal_IAudioClient2_GetCurrentPadding(mal_IAudioClient2* pThis, mal_uint32* pNumPaddingFrames)           { return pThis->lpVtbl->GetCurrentPadding(pThis, pNumPaddingFrames); }
HRESULT mal_IAudioClient2_IsFormatSupported(mal_IAudioClient2* pThis, MAL_AUDCLNT_SHAREMODE shareMode, const WAVEFORMATEX* pFormat, WAVEFORMATEX** ppClosestMatch) { return pThis->lpVtbl->IsFormatSupported(pThis, shareMode, pFormat, ppClosestMatch); }
HRESULT mal_IAudioClient2_GetMixFormat(mal_IAudioClient2* pThis, WAVEFORMATEX** ppDeviceFormat)            { return pThis->lpVtbl->GetMixFormat(pThis, ppDeviceFormat); }
HRESULT mal_IAudioClient2_GetDevicePeriod(mal_IAudioClient2* pThis, MAL_REFERENCE_TIME* pDefaultDevicePeriod, MAL_REFERENCE_TIME* pMinimumDevicePeriod) { return pThis->lpVtbl->GetDevicePeriod(pThis, pDefaultDevicePeriod, pMinimumDevicePeriod); }
HRESULT mal_IAudioClient2_Start(mal_IAudioClient2* pThis)                                                  { return pThis->lpVtbl->Start(pThis); }
HRESULT mal_IAudioClient2_Stop(mal_IAudioClient2* pThis)                                                   { return pThis->lpVtbl->Stop(pThis); }
HRESULT mal_IAudioClient2_Reset(mal_IAudioClient2* pThis)                                                  { return pThis->lpVtbl->Reset(pThis); }
HRESULT mal_IAudioClient2_SetEventHandle(mal_IAudioClient2* pThis, HANDLE eventHandle)                     { return pThis->lpVtbl->SetEventHandle(pThis, eventHandle); }
HRESULT mal_IAudioClient2_GetService(mal_IAudioClient2* pThis, const IID* const riid, void** pp)           { return pThis->lpVtbl->GetService(pThis, riid, pp); }
HRESULT mal_IAudioClient2_IsOffloadCapable(mal_IAudioClient2* pThis, MAL_AUDIO_STREAM_CATEGORY category, BOOL* pOffloadCapable) { return pThis->lpVtbl->IsOffloadCapable(pThis, category, pOffloadCapable); }
HRESULT mal_IAudioClient2_SetClientProperties(mal_IAudioClient2* pThis, const mal_AudioClientProperties* pProperties)           { return pThis->lpVtbl->SetClientProperties(pThis, pProperties); }
HRESULT mal_IAudioClient2_GetBufferSizeLimits(mal_IAudioClient2* pThis, const WAVEFORMATEX* pFormat, BOOL eventDriven, MAL_REFERENCE_TIME* pMinBufferDuration, MAL_REFERENCE_TIME* pMaxBufferDuration) { return pThis->lpVtbl->GetBufferSizeLimits(pThis, pFormat, eventDriven, pMinBufferDuration, pMaxBufferDuration); }


// IAudioClient3
typedef struct
{
    // IUnknown
    HRESULT (STDMETHODCALLTYPE * QueryInterface)(mal_IAudioClient3* pThis, const IID* const riid, void** ppObject);
    ULONG   (STDMETHODCALLTYPE * AddRef)        (mal_IAudioClient3* pThis);
    ULONG   (STDMETHODCALLTYPE * Release)       (mal_IAudioClient3* pThis);

    // IAudioClient
    HRESULT (STDMETHODCALLTYPE * Initialize)       (mal_IAudioClient3* pThis, MAL_AUDCLNT_SHAREMODE shareMode, DWORD streamFlags, MAL_REFERENCE_TIME bufferDuration, MAL_REFERENCE_TIME periodicity, const WAVEFORMATEX* pFormat, const GUID* pAudioSessionGuid);
    HRESULT (STDMETHODCALLTYPE * GetBufferSize)    (mal_IAudioClient3* pThis, mal_uint32* pNumBufferFrames);
    HRESULT (STDMETHODCALLTYPE * GetStreamLatency) (mal_IAudioClient3* pThis, MAL_REFERENCE_TIME* pLatency);
    HRESULT (STDMETHODCALLTYPE * GetCurrentPadding)(mal_IAudioClient3* pThis, mal_uint32* pNumPaddingFrames);
    HRESULT (STDMETHODCALLTYPE * IsFormatSupported)(mal_IAudioClient3* pThis, MAL_AUDCLNT_SHAREMODE shareMode, const WAVEFORMATEX* pFormat, WAVEFORMATEX** ppClosestMatch);
    HRESULT (STDMETHODCALLTYPE * GetMixFormat)     (mal_IAudioClient3* pThis, WAVEFORMATEX** ppDeviceFormat);
    HRESULT (STDMETHODCALLTYPE * GetDevicePeriod)  (mal_IAudioClient3* pThis, MAL_REFERENCE_TIME* pDefaultDevicePeriod, MAL_REFERENCE_TIME* pMinimumDevicePeriod);
    HRESULT (STDMETHODCALLTYPE * Start)            (mal_IAudioClient3* pThis);
    HRESULT (STDMETHODCALLTYPE * Stop)             (mal_IAudioClient3* pThis);
    HRESULT (STDMETHODCALLTYPE * Reset)            (mal_IAudioClient3* pThis);
    HRESULT (STDMETHODCALLTYPE * SetEventHandle)   (mal_IAudioClient3* pThis, HANDLE eventHandle);
    HRESULT (STDMETHODCALLTYPE * GetService)       (mal_IAudioClient3* pThis, const IID* const riid, void** pp);

    // IAudioClient2
    HRESULT (STDMETHODCALLTYPE * IsOffloadCapable)   (mal_IAudioClient3* pThis, MAL_AUDIO_STREAM_CATEGORY category, BOOL* pOffloadCapable);
    HRESULT (STDMETHODCALLTYPE * SetClientProperties)(mal_IAudioClient3* pThis, const mal_AudioClientProperties* pProperties);
    HRESULT (STDMETHODCALLTYPE * GetBufferSizeLimits)(mal_IAudioClient3* pThis, const WAVEFORMATEX* pFormat, BOOL eventDriven, MAL_REFERENCE_TIME* pMinBufferDuration, MAL_REFERENCE_TIME* pMaxBufferDuration);

    // IAudioClient3
    HRESULT (STDMETHODCALLTYPE * GetSharedModeEnginePeriod)       (mal_IAudioClient3* pThis, const WAVEFORMATEX* pFormat, UINT32* pDefaultPeriodInFrames, UINT32* pFundamentalPeriodInFrames, UINT32* pMinPeriodInFrames, UINT32* pMaxPeriodInFrames);
    HRESULT (STDMETHODCALLTYPE * GetCurrentSharedModeEnginePeriod)(mal_IAudioClient3* pThis, WAVEFORMATEX** ppFormat, UINT32* pCurrentPeriodInFrames);
    HRESULT (STDMETHODCALLTYPE * InitializeSharedAudioStream)     (mal_IAudioClient3* pThis, DWORD streamFlags, UINT32 periodInFrames, const WAVEFORMATEX* pFormat, const GUID* pAudioSessionGuid);
} mal_IAudioClient3Vtbl;
struct mal_IAudioClient3
{
    mal_IAudioClient3Vtbl* lpVtbl;
};
HRESULT mal_IAudioClient3_QueryInterface(mal_IAudioClient3* pThis, const IID* const riid, void** ppObject) { return pThis->lpVtbl->QueryInterface(pThis, riid, ppObject); }
ULONG   mal_IAudioClient3_AddRef(mal_IAudioClient3* pThis)                                                 { return pThis->lpVtbl->AddRef(pThis); }
ULONG   mal_IAudioClient3_Release(mal_IAudioClient3* pThis)                                                { return pThis->lpVtbl->Release(pThis); }
HRESULT mal_IAudioClient3_Initialize(mal_IAudioClient3* pThis, MAL_AUDCLNT_SHAREMODE shareMode, DWORD streamFlags, MAL_REFERENCE_TIME bufferDuration, MAL_REFERENCE_TIME periodicity, const WAVEFORMATEX* pFormat, const GUID* pAudioSessionGuid) { return pThis->lpVtbl->Initialize(pThis, shareMode, streamFlags, bufferDuration, periodicity, pFormat, pAudioSessionGuid); }
HRESULT mal_IAudioClient3_GetBufferSize(mal_IAudioClient3* pThis, mal_uint32* pNumBufferFrames)                { return pThis->lpVtbl->GetBufferSize(pThis, pNumBufferFrames); }
HRESULT mal_IAudioClient3_GetStreamLatency(mal_IAudioClient3* pThis, MAL_REFERENCE_TIME* pLatency)             { return pThis->lpVtbl->GetStreamLatency(pThis, pLatency); }
HRESULT mal_IAudioClient3_GetCurrentPadding(mal_IAudioClient3* pThis, mal_uint32* pNumPaddingFrames)           { return pThis->lpVtbl->GetCurrentPadding(pThis, pNumPaddingFrames); }
HRESULT mal_IAudioClient3_IsFormatSupported(mal_IAudioClient3* pThis, MAL_AUDCLNT_SHAREMODE shareMode, const WAVEFORMATEX* pFormat, WAVEFORMATEX** ppClosestMatch) { return pThis->lpVtbl->IsFormatSupported(pThis, shareMode, pFormat, ppClosestMatch); }
HRESULT mal_IAudioClient3_GetMixFormat(mal_IAudioClient3* pThis, WAVEFORMATEX** ppDeviceFormat)            { return pThis->lpVtbl->GetMixFormat(pThis, ppDeviceFormat); }
HRESULT mal_IAudioClient3_GetDevicePeriod(mal_IAudioClient3* pThis, MAL_REFERENCE_TIME* pDefaultDevicePeriod, MAL_REFERENCE_TIME* pMinimumDevicePeriod) { return pThis->lpVtbl->GetDevicePeriod(pThis, pDefaultDevicePeriod, pMinimumDevicePeriod); }
HRESULT mal_IAudioClient3_Start(mal_IAudioClient3* pThis)                                                  { return pThis->lpVtbl->Start(pThis); }
HRESULT mal_IAudioClient3_Stop(mal_IAudioClient3* pThis)                                                   { return pThis->lpVtbl->Stop(pThis); }
HRESULT mal_IAudioClient3_Reset(mal_IAudioClient3* pThis)                                                  { return pThis->lpVtbl->Reset(pThis); }
HRESULT mal_IAudioClient3_SetEventHandle(mal_IAudioClient3* pThis, HANDLE eventHandle)                     { return pThis->lpVtbl->SetEventHandle(pThis, eventHandle); }
HRESULT mal_IAudioClient3_GetService(mal_IAudioClient3* pThis, const IID* const riid, void** pp)           { return pThis->lpVtbl->GetService(pThis, riid, pp); }
HRESULT mal_IAudioClient3_IsOffloadCapable(mal_IAudioClient3* pThis, MAL_AUDIO_STREAM_CATEGORY category, BOOL* pOffloadCapable) { return pThis->lpVtbl->IsOffloadCapable(pThis, category, pOffloadCapable); }
HRESULT mal_IAudioClient3_SetClientProperties(mal_IAudioClient3* pThis, const mal_AudioClientProperties* pProperties)           { return pThis->lpVtbl->SetClientProperties(pThis, pProperties); }
HRESULT mal_IAudioClient3_GetBufferSizeLimits(mal_IAudioClient3* pThis, const WAVEFORMATEX* pFormat, BOOL eventDriven, MAL_REFERENCE_TIME* pMinBufferDuration, MAL_REFERENCE_TIME* pMaxBufferDuration) { return pThis->lpVtbl->GetBufferSizeLimits(pThis, pFormat, eventDriven, pMinBufferDuration, pMaxBufferDuration); }
HRESULT mal_IAudioClient3_GetSharedModeEnginePeriod(mal_IAudioClient3* pThis, const WAVEFORMATEX* pFormat, UINT32* pDefaultPeriodInFrames, UINT32* pFundamentalPeriodInFrames, UINT32* pMinPeriodInFrames, UINT32* pMaxPeriodInFrames) { return pThis->lpVtbl->GetSharedModeEnginePeriod(pThis, pFormat, pDefaultPeriodInFrames, pFundamentalPeriodInFrames, pMinPeriodInFrames, pMaxPeriodInFrames); }
HRESULT mal_IAudioClient3_GetCurrentSharedModeEnginePeriod(mal_IAudioClient3* pThis, WAVEFORMATEX** ppFormat, UINT32* pCurrentPeriodInFrames) { return pThis->lpVtbl->GetCurrentSharedModeEnginePeriod(pThis, ppFormat, pCurrentPeriodInFrames); }
HRESULT mal_IAudioClient3_InitializeSharedAudioStream(mal_IAudioClient3* pThis, DWORD streamFlags, UINT32 periodInFrames, const WAVEFORMATEX* pFormat, const GUID* pAudioSessionGUID) { return pThis->lpVtbl->InitializeSharedAudioStream(pThis, streamFlags, periodInFrames, pFormat, pAudioSessionGUID); }


// IAudioRenderClient
typedef struct
{
    // IUnknown
    HRESULT (STDMETHODCALLTYPE * QueryInterface)(mal_IAudioRenderClient* pThis, const IID* const riid, void** ppObject);
    ULONG   (STDMETHODCALLTYPE * AddRef)        (mal_IAudioRenderClient* pThis);
    ULONG   (STDMETHODCALLTYPE * Release)       (mal_IAudioRenderClient* pThis);

    // IAudioRenderClient
    HRESULT (STDMETHODCALLTYPE * GetBuffer)    (mal_IAudioRenderClient* pThis, mal_uint32 numFramesRequested, BYTE** ppData);
    HRESULT (STDMETHODCALLTYPE * ReleaseBuffer)(mal_IAudioRenderClient* pThis, mal_uint32 numFramesWritten, DWORD dwFlags);
} mal_IAudioRenderClientVtbl;
struct mal_IAudioRenderClient
{
    mal_IAudioRenderClientVtbl* lpVtbl;
};
HRESULT mal_IAudioRenderClient_QueryInterface(mal_IAudioRenderClient* pThis, const IID* const riid, void** ppObject) { return pThis->lpVtbl->QueryInterface(pThis, riid, ppObject); }
ULONG   mal_IAudioRenderClient_AddRef(mal_IAudioRenderClient* pThis)                                                 { return pThis->lpVtbl->AddRef(pThis); }
ULONG   mal_IAudioRenderClient_Release(mal_IAudioRenderClient* pThis)                                                { return pThis->lpVtbl->Release(pThis); }
HRESULT mal_IAudioRenderClient_GetBuffer(mal_IAudioRenderClient* pThis, mal_uint32 numFramesRequested, BYTE** ppData)    { return pThis->lpVtbl->GetBuffer(pThis, numFramesRequested, ppData); }
HRESULT mal_IAudioRenderClient_ReleaseBuffer(mal_IAudioRenderClient* pThis, mal_uint32 numFramesWritten, DWORD dwFlags)  { return pThis->lpVtbl->ReleaseBuffer(pThis, numFramesWritten, dwFlags); }


// IAudioCaptureClient
typedef struct
{
    // IUnknown
    HRESULT (STDMETHODCALLTYPE * QueryInterface)(mal_IAudioCaptureClient* pThis, const IID* const riid, void** ppObject);
    ULONG   (STDMETHODCALLTYPE * AddRef)        (mal_IAudioCaptureClient* pThis);
    ULONG   (STDMETHODCALLTYPE * Release)       (mal_IAudioCaptureClient* pThis);

    // IAudioRenderClient
    HRESULT (STDMETHODCALLTYPE * GetBuffer)        (mal_IAudioCaptureClient* pThis, BYTE** ppData, mal_uint32* pNumFramesToRead, DWORD* pFlags, mal_uint64* pDevicePosition, mal_uint64* pQPCPosition);
    HRESULT (STDMETHODCALLTYPE * ReleaseBuffer)    (mal_IAudioCaptureClient* pThis, mal_uint32 numFramesRead);
    HRESULT (STDMETHODCALLTYPE * GetNextPacketSize)(mal_IAudioCaptureClient* pThis, mal_uint32* pNumFramesInNextPacket);
} mal_IAudioCaptureClientVtbl;
struct mal_IAudioCaptureClient
{
    mal_IAudioCaptureClientVtbl* lpVtbl;
};
HRESULT mal_IAudioCaptureClient_QueryInterface(mal_IAudioCaptureClient* pThis, const IID* const riid, void** ppObject) { return pThis->lpVtbl->QueryInterface(pThis, riid, ppObject); }
ULONG   mal_IAudioCaptureClient_AddRef(mal_IAudioCaptureClient* pThis)                                                 { return pThis->lpVtbl->AddRef(pThis); }
ULONG   mal_IAudioCaptureClient_Release(mal_IAudioCaptureClient* pThis)                                                { return pThis->lpVtbl->Release(pThis); }
HRESULT mal_IAudioCaptureClient_GetBuffer(mal_IAudioCaptureClient* pThis, BYTE** ppData, mal_uint32* pNumFramesToRead, DWORD* pFlags, mal_uint64* pDevicePosition, mal_uint64* pQPCPosition) { return pThis->lpVtbl->GetBuffer(pThis, ppData, pNumFramesToRead, pFlags, pDevicePosition, pQPCPosition); }
HRESULT mal_IAudioCaptureClient_ReleaseBuffer(mal_IAudioCaptureClient* pThis, mal_uint32 numFramesRead)                    { return pThis->lpVtbl->ReleaseBuffer(pThis, numFramesRead); }
HRESULT mal_IAudioCaptureClient_GetNextPacketSize(mal_IAudioCaptureClient* pThis, mal_uint32* pNumFramesInNextPacket)      { return pThis->lpVtbl->GetNextPacketSize(pThis, pNumFramesInNextPacket); }

#ifndef MAL_WIN32_DESKTOP
#include <mmdeviceapi.h>
typedef struct mal_completion_handler_uwp mal_completion_handler_uwp;

typedef struct
{
    // IUnknown
    HRESULT (STDMETHODCALLTYPE * QueryInterface)(mal_completion_handler_uwp* pThis, const IID* const riid, void** ppObject);
    ULONG   (STDMETHODCALLTYPE * AddRef)        (mal_completion_handler_uwp* pThis);
    ULONG   (STDMETHODCALLTYPE * Release)       (mal_completion_handler_uwp* pThis);

    // IActivateAudioInterfaceCompletionHandler
    HRESULT (STDMETHODCALLTYPE * ActivateCompleted)(mal_completion_handler_uwp* pThis, mal_IActivateAudioInterfaceAsyncOperation* pActivateOperation);
} mal_completion_handler_uwp_vtbl;
struct mal_completion_handler_uwp
{
    mal_completion_handler_uwp_vtbl* lpVtbl;
    mal_uint32 counter;
    HANDLE hEvent;
};

HRESULT STDMETHODCALLTYPE mal_completion_handler_uwp_QueryInterface(mal_completion_handler_uwp* pThis, const IID* const riid, void** ppObject)
{
    // We need to "implement" IAgileObject which is just an indicator that's used internally by WASAPI for some multithreading management. To
    // "implement" this, we just make sure we return pThis when the IAgileObject is requested.
    if (!mal_is_guid_equal(riid, &MAL_IID_IUnknown) && !mal_is_guid_equal(riid, &MAL_IID_IActivateAudioInterfaceCompletionHandler) && !mal_is_guid_equal(riid, &MAL_IID_IAgileObject)) {
        *ppObject = NULL;
        return E_NOINTERFACE;
    }

    // Getting here means the IID is IUnknown or IMMNotificationClient.
    *ppObject = (void*)pThis;
    ((mal_completion_handler_uwp_vtbl*)pThis->lpVtbl)->AddRef(pThis);
    return S_OK;
}

ULONG STDMETHODCALLTYPE mal_completion_handler_uwp_AddRef(mal_completion_handler_uwp* pThis)
{
    return (ULONG)mal_atomic_increment_32(&pThis->counter);
}

ULONG STDMETHODCALLTYPE mal_completion_handler_uwp_Release(mal_completion_handler_uwp* pThis)
{
    mal_uint32 newRefCount = mal_atomic_decrement_32(&pThis->counter);
    if (newRefCount == 0) {
        return 0;   // We don't free anything here because we never allocate the object on the heap.
    }

    return (ULONG)newRefCount;
}

HRESULT STDMETHODCALLTYPE mal_completion_handler_uwp_ActivateCompleted(mal_completion_handler_uwp* pThis, mal_IActivateAudioInterfaceAsyncOperation* pActivateOperation)
{
    (void)pActivateOperation;
    SetEvent(pThis->hEvent);
    return S_OK;
}


static mal_completion_handler_uwp_vtbl g_malCompletionHandlerVtblInstance = {
    mal_completion_handler_uwp_QueryInterface,
    mal_completion_handler_uwp_AddRef,
    mal_completion_handler_uwp_Release,
    mal_completion_handler_uwp_ActivateCompleted
};

mal_result mal_completion_handler_uwp_init(mal_completion_handler_uwp* pHandler)
{
    mal_assert(pHandler != NULL);
    mal_zero_object(pHandler);

    pHandler->lpVtbl = &g_malCompletionHandlerVtblInstance;
    pHandler->counter = 1;
    pHandler->hEvent = CreateEventA(NULL, FALSE, FALSE, NULL);
    if (pHandler->hEvent == NULL) {
        return MAL_ERROR;
    }

    return MAL_SUCCESS;
}

void mal_completion_handler_uwp_uninit(mal_completion_handler_uwp* pHandler)
{
    if (pHandler->hEvent != NULL) {
        CloseHandle(pHandler->hEvent);
    }
}

void mal_completion_handler_uwp_wait(mal_completion_handler_uwp* pHandler)
{
    WaitForSingleObject(pHandler->hEvent, INFINITE);
}
#endif  // !MAL_WIN32_DESKTOP

// We need a virtual table for our notification client object that's used for detecting changes to the default device.
#ifdef MAL_WIN32_DESKTOP
HRESULT STDMETHODCALLTYPE mal_IMMNotificationClient_QueryInterface(mal_IMMNotificationClient* pThis, const IID* const riid, void** ppObject)
{
    // We care about two interfaces - IUnknown and IMMNotificationClient. If the requested IID is something else
    // we just return E_NOINTERFACE. Otherwise we need to increment the reference counter and return S_OK.
    if (!mal_is_guid_equal(riid, &MAL_IID_IUnknown) && !mal_is_guid_equal(riid, &MAL_IID_IMMNotificationClient)) {
        *ppObject = NULL;
        return E_NOINTERFACE;
    }

    // Getting here means the IID is IUnknown or IMMNotificationClient.
    *ppObject = (void*)pThis;
    ((mal_IMMNotificationClientVtbl*)pThis->lpVtbl)->AddRef(pThis);
    return S_OK;
}

ULONG STDMETHODCALLTYPE mal_IMMNotificationClient_AddRef(mal_IMMNotificationClient* pThis)
{
    return (ULONG)mal_atomic_increment_32(&pThis->counter);
}

ULONG STDMETHODCALLTYPE mal_IMMNotificationClient_Release(mal_IMMNotificationClient* pThis)
{
    mal_uint32 newRefCount = mal_atomic_decrement_32(&pThis->counter);
    if (newRefCount == 0) {
        return 0;   // We don't free anything here because we never allocate the object on the heap.
    }

    return (ULONG)newRefCount;
}


HRESULT STDMETHODCALLTYPE mal_IMMNotificationClient_OnDeviceStateChanged(mal_IMMNotificationClient* pThis, LPCWSTR pDeviceID, DWORD dwNewState)
{
#ifdef MAL_DEBUG_OUTPUT
    printf("IMMNotificationClient_OnDeviceStateChanged(pDeviceID=%S, dwNewState=%u)\n", pDeviceID, (unsigned int)dwNewState);
#endif

    (void)pThis;
    (void)pDeviceID;
    (void)dwNewState;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE mal_IMMNotificationClient_OnDeviceAdded(mal_IMMNotificationClient* pThis, LPCWSTR pDeviceID)
{
#ifdef MAL_DEBUG_OUTPUT
    printf("IMMNotificationClient_OnDeviceAdded(pDeviceID=%S)\n", pDeviceID);
#endif

    // We don't need to worry about this event for our purposes.
    (void)pThis;
    (void)pDeviceID;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE mal_IMMNotificationClient_OnDeviceRemoved(mal_IMMNotificationClient* pThis, LPCWSTR pDeviceID)
{
#ifdef MAL_DEBUG_OUTPUT
    printf("IMMNotificationClient_OnDeviceRemoved(pDeviceID=%S)\n", pDeviceID);
#endif

    // We don't need to worry about this event for our purposes.
    (void)pThis;
    (void)pDeviceID;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE mal_IMMNotificationClient_OnDefaultDeviceChanged(mal_IMMNotificationClient* pThis, mal_EDataFlow dataFlow, mal_ERole role, LPCWSTR pDefaultDeviceID)
{
#ifdef MAL_DEBUG_OUTPUT
    printf("IMMNotificationClient_OnDefaultDeviceChanged(dataFlow=%d, role=%d, pDefaultDeviceID=%S)\n", dataFlow, role, pDefaultDeviceID);
#endif

    // We only ever use the eConsole role in mini_al.
    if (role != mal_eConsole) {
        return S_OK;
    }

    // We only care about devices with the same data flow and role as the current device.
    if ((pThis->pDevice->type == mal_device_type_playback && dataFlow != mal_eRender ) ||
        (pThis->pDevice->type == mal_device_type_capture  && dataFlow != mal_eCapture)) {
        return S_OK;
    }

    // Not currently supporting automatic stream routing in exclusive mode. This is not working correctly on my machine due to
    // AUDCLNT_E_DEVICE_IN_USE errors when reinitializing the device. If this is a bug in mini_al, we can try re-enabling this once
    // it's fixed.
    if (pThis->pDevice->exclusiveMode) {
        return S_OK;
    }

    // We don't change the device here - we change it in the worker thread to keep synchronization simple. To this I'm just setting a flag to
    // indicate that the default device has changed.
    mal_atomic_exchange_32(&pThis->pDevice->wasapi.hasDefaultDeviceChanged, MAL_TRUE);
    SetEvent(pThis->pDevice->wasapi.hBreakEvent);   // <-- The main loop will be waiting on some events. We want to break from this wait ASAP so we can change the device as quickly as possible.

    
    (void)pDefaultDeviceID;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE mal_IMMNotificationClient_OnPropertyValueChanged(mal_IMMNotificationClient* pThis, LPCWSTR pDeviceID, const PROPERTYKEY key)
{
#ifdef MAL_DEBUG_OUTPUT
    printf("IMMNotificationClient_OnPropertyValueChanged(pDeviceID=%S)\n", pDeviceID);
#endif

    (void)pThis;
    (void)pDeviceID;
    (void)key;
    return S_OK;
}

static mal_IMMNotificationClientVtbl g_malNotificationCientVtbl = {
    mal_IMMNotificationClient_QueryInterface,
    mal_IMMNotificationClient_AddRef,
    mal_IMMNotificationClient_Release,
    mal_IMMNotificationClient_OnDeviceStateChanged,
    mal_IMMNotificationClient_OnDeviceAdded,
    mal_IMMNotificationClient_OnDeviceRemoved,
    mal_IMMNotificationClient_OnDefaultDeviceChanged,
    mal_IMMNotificationClient_OnPropertyValueChanged
};
#endif  // MAL_WIN32_DESKTOP

mal_bool32 mal_context_is_device_id_equal__wasapi(mal_context* pContext, const mal_device_id* pID0, const mal_device_id* pID1)
{
    mal_assert(pContext != NULL);
    mal_assert(pID0 != NULL);
    mal_assert(pID1 != NULL);
    (void)pContext;

    return memcmp(pID0->wasapi, pID1->wasapi, sizeof(pID0->wasapi)) == 0;
}

void mal_set_device_info_from_WAVEFORMATEX(const WAVEFORMATEX* pWF, mal_device_info* pInfo)
{
    mal_assert(pWF != NULL);
    mal_assert(pInfo != NULL);

    pInfo->formatCount   = 1;
    pInfo->formats[0]    = mal_format_from_WAVEFORMATEX(pWF);
    pInfo->minChannels   = pWF->nChannels;
    pInfo->maxChannels   = pWF->nChannels;
    pInfo->minSampleRate = pWF->nSamplesPerSec;
    pInfo->maxSampleRate = pWF->nSamplesPerSec;
}

#ifndef MAL_WIN32_DESKTOP
mal_result mal_context_get_IAudioClient_UWP__wasapi(mal_context* pContext, mal_device_type deviceType, const mal_device_id* pDeviceID, mal_IAudioClient** ppAudioClient, mal_IUnknown** ppActivatedInterface)
{
    mal_assert(pContext != NULL);
    mal_assert(ppAudioClient != NULL);

    mal_IActivateAudioInterfaceAsyncOperation *pAsyncOp = NULL;
    mal_completion_handler_uwp completionHandler;

    IID iid;
    if (pDeviceID != NULL) {
        mal_copy_memory(&iid, pDeviceID->wasapi, sizeof(iid));
    } else {
        if (deviceType == mal_device_type_playback) {
            iid = MAL_IID_DEVINTERFACE_AUDIO_RENDER;
        } else {
            iid = MAL_IID_DEVINTERFACE_AUDIO_CAPTURE;
        }
    }

    LPOLESTR iidStr;
#if defined(__cplusplus)
    HRESULT hr = StringFromIID(iid, &iidStr);
#else
    HRESULT hr = StringFromIID(&iid, &iidStr);
#endif
    if (FAILED(hr)) {
        return mal_context_post_error(pContext, NULL, MAL_LOG_LEVEL_ERROR, "[WASAPI] Failed to convert device IID to string for ActivateAudioInterfaceAsync(). Out of memory.", MAL_OUT_OF_MEMORY);
    }

    mal_result result = mal_completion_handler_uwp_init(&completionHandler);
    if (result != MAL_SUCCESS) {
        mal_CoTaskMemFree(pContext, iidStr);
        return mal_context_post_error(pContext, NULL, MAL_LOG_LEVEL_ERROR, "[WASAPI] Failed to create event for waiting for ActivateAudioInterfaceAsync().", MAL_FAILED_TO_OPEN_BACKEND_DEVICE);
    }

#if defined(__cplusplus)
    hr = ActivateAudioInterfaceAsync(iidStr, MAL_IID_IAudioClient, NULL, (IActivateAudioInterfaceCompletionHandler*)&completionHandler, (IActivateAudioInterfaceAsyncOperation**)&pAsyncOp);
#else
    hr = ActivateAudioInterfaceAsync(iidStr, &MAL_IID_IAudioClient, NULL, (IActivateAudioInterfaceCompletionHandler*)&completionHandler, (IActivateAudioInterfaceAsyncOperation**)&pAsyncOp);
#endif
    if (FAILED(hr)) {
        mal_completion_handler_uwp_uninit(&completionHandler);
        mal_CoTaskMemFree(pContext, iidStr);
        return mal_context_post_error(pContext, NULL, MAL_LOG_LEVEL_ERROR, "[WASAPI] ActivateAudioInterfaceAsync() failed.", MAL_FAILED_TO_OPEN_BACKEND_DEVICE);
    }

    mal_CoTaskMemFree(pContext, iidStr);

    // Wait for the async operation for finish.
    mal_completion_handler_uwp_wait(&completionHandler);
    mal_completion_handler_uwp_uninit(&completionHandler);

    HRESULT activateResult;
    mal_IUnknown* pActivatedInterface;
    hr = mal_IActivateAudioInterfaceAsyncOperation_GetActivateResult(pAsyncOp, &activateResult, &pActivatedInterface);
    mal_IActivateAudioInterfaceAsyncOperation_Release(pAsyncOp);

    if (FAILED(hr) || FAILED(activateResult)) {
        return mal_context_post_error(pContext, NULL, MAL_LOG_LEVEL_ERROR, "[WASAPI] Failed to activate device.", MAL_FAILED_TO_OPEN_BACKEND_DEVICE);
    }

    // Here is where we grab the IAudioClient interface.
    hr = mal_IUnknown_QueryInterface(pActivatedInterface, &MAL_IID_IAudioClient, (void**)ppAudioClient);
    if (FAILED(hr)) {
        return mal_context_post_error(pContext, NULL, MAL_LOG_LEVEL_ERROR, "[WASAPI] Failed to query IAudioClient interface.", MAL_FAILED_TO_OPEN_BACKEND_DEVICE);
    }

    if (ppActivatedInterface) {
        *ppActivatedInterface = pActivatedInterface;
    } else {
        mal_IUnknown_Release(pActivatedInterface);
    }

    return MAL_SUCCESS;
}
#endif

mal_result mal_context_get_device_info_from_IAudioClient__wasapi(mal_context* pContext, /*mal_IMMDevice**/void* pMMDevice, mal_IAudioClient* pAudioClient, mal_share_mode shareMode, mal_device_info* pInfo)
{
    mal_assert(pAudioClient != NULL);
    mal_assert(pInfo != NULL);

    // We use a different technique to retrieve the device information depending on whether or not we are using shared or exclusive mode.
    if (shareMode == mal_share_mode_shared) {
        // Shared Mode. We use GetMixFormat() here.
        WAVEFORMATEX* pWF = NULL;
        HRESULT hr = mal_IAudioClient_GetMixFormat((mal_IAudioClient*)pAudioClient, (WAVEFORMATEX**)&pWF);
        if (SUCCEEDED(hr)) {
            mal_set_device_info_from_WAVEFORMATEX(pWF, pInfo);
            return MAL_SUCCESS;
        } else {
            return mal_context_post_error(pContext, NULL, MAL_LOG_LEVEL_ERROR, "[WASAPI] Failed to retrieve mix format for device info retrieval.", MAL_FAILED_TO_OPEN_BACKEND_DEVICE);
        }
    } else {
        // Exlcusive Mode. We repeatedly call IsFormatSupported() here. This is not currently support on UWP.
#ifdef MAL_WIN32_DESKTOP
        // The first thing to do is get the format from PKEY_AudioEngine_DeviceFormat. This should give us a channel count we assume is
        // correct which will simplify our searching.
        mal_IPropertyStore *pProperties;
        HRESULT hr = mal_IMMDevice_OpenPropertyStore((mal_IMMDevice*)pMMDevice, STGM_READ, &pProperties);
        if (SUCCEEDED(hr)) {
            PROPVARIANT var;
            mal_PropVariantInit(&var);

            hr = mal_IPropertyStore_GetValue(pProperties, &MAL_PKEY_AudioEngine_DeviceFormat, &var);
            if (SUCCEEDED(hr)) {
                WAVEFORMATEX* pWF = (WAVEFORMATEX*)var.blob.pBlobData;
                mal_set_device_info_from_WAVEFORMATEX(pWF, pInfo);

                // In my testing, the format returned by PKEY_AudioEngine_DeviceFormat is suitable for exclusive mode so we check this format
                // first. If this fails, fall back to a search.
                hr = mal_IAudioClient_IsFormatSupported((mal_IAudioClient*)pAudioClient, MAL_AUDCLNT_SHAREMODE_EXCLUSIVE, pWF, NULL);
                mal_PropVariantClear(pContext, &var);

                if (FAILED(hr)) {
                    // The format returned by PKEY_AudioEngine_DeviceFormat is not supported, so fall back to a search. We assume the channel
                    // count returned by MAL_PKEY_AudioEngine_DeviceFormat is valid and correct. For simplicity we're only returning one format.
                    mal_uint32 channels = pInfo->minChannels;

                    mal_format formatsToSearch[] = {
                        mal_format_s16,
                        mal_format_s24,
                        //mal_format_s24_32,
                        mal_format_f32,
                        mal_format_s32,
                        mal_format_u8
                    };

                    mal_channel defaultChannelMap[MAL_MAX_CHANNELS];
                    mal_get_standard_channel_map(mal_standard_channel_map_microsoft, channels, defaultChannelMap);

                    WAVEFORMATEXTENSIBLE wf;
                    mal_zero_object(&wf);
                    wf.Format.cbSize     = sizeof(wf);
                    wf.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
                    wf.Format.nChannels  = (WORD)channels;
                    wf.dwChannelMask     = mal_channel_map_to_channel_mask__win32(defaultChannelMap, channels);

                    mal_bool32 found = MAL_FALSE;
                    for (mal_uint32 iFormat = 0; iFormat < mal_countof(formatsToSearch); ++iFormat) {
                        mal_format format = formatsToSearch[iFormat];

                        wf.Format.wBitsPerSample       = (WORD)mal_get_bytes_per_sample(format)*8;
                        wf.Format.nBlockAlign          = (wf.Format.nChannels * wf.Format.wBitsPerSample) / 8;
                        wf.Format.nAvgBytesPerSec      = wf.Format.nBlockAlign * wf.Format.nSamplesPerSec;
                        wf.Samples.wValidBitsPerSample = /*(format == mal_format_s24_32) ? 24 :*/ wf.Format.wBitsPerSample;
                        if (format == mal_format_f32) {
                            wf.SubFormat = MAL_GUID_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
                        } else {
                            wf.SubFormat = MAL_GUID_KSDATAFORMAT_SUBTYPE_PCM;
                        }

                        for (mal_uint32 iSampleRate = 0; iSampleRate < mal_countof(g_malStandardSampleRatePriorities); ++iSampleRate) {
                            wf.Format.nSamplesPerSec = g_malStandardSampleRatePriorities[iSampleRate];

                            hr = mal_IAudioClient_IsFormatSupported((mal_IAudioClient*)pAudioClient, MAL_AUDCLNT_SHAREMODE_EXCLUSIVE, (WAVEFORMATEX*)&wf, NULL);
                            if (SUCCEEDED(hr)) {
                                mal_set_device_info_from_WAVEFORMATEX((WAVEFORMATEX*)&wf, pInfo);
                                found = MAL_TRUE;
                                break;
                            }
                        }

                        if (found) {
                            break;
                        }
                    }

                    if (!found) {
                        mal_IPropertyStore_Release(pProperties);
                        return mal_context_post_error(pContext, NULL, MAL_LOG_LEVEL_ERROR, "[WASAPI] Failed to find suitable device format for device info retrieval.", MAL_FAILED_TO_OPEN_BACKEND_DEVICE);
                    }
                }
            } else {
                mal_IPropertyStore_Release(pProperties);
                return mal_context_post_error(pContext, NULL, MAL_LOG_LEVEL_ERROR, "[WASAPI] Failed to retrieve device format for device info retrieval.", MAL_FAILED_TO_OPEN_BACKEND_DEVICE);
            }
        } else {
            return mal_context_post_error(pContext, NULL, MAL_LOG_LEVEL_ERROR, "[WASAPI] Failed to open property store for device info retrieval.", MAL_FAILED_TO_OPEN_BACKEND_DEVICE);
        }

        return MAL_SUCCESS;
#else
        // Exclusive mode not fully supported in UWP right now.
        return MAL_ERROR;
#endif
    }
}

#ifdef MAL_WIN32_DESKTOP
mal_result mal_context_get_MMDevice__wasapi(mal_context* pContext, mal_device_type deviceType, const mal_device_id* pDeviceID, mal_IMMDevice** ppMMDevice)
{
    mal_assert(pContext != NULL);
    mal_assert(ppMMDevice != NULL);

    mal_IMMDeviceEnumerator* pDeviceEnumerator;
    HRESULT hr = mal_CoCreateInstance(pContext, MAL_CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, MAL_IID_IMMDeviceEnumerator, (void**)&pDeviceEnumerator);
    if (FAILED(hr)) {
        return mal_context_post_error(pContext, NULL, MAL_LOG_LEVEL_ERROR, "[WASAPI] Failed to create IMMDeviceEnumerator.", MAL_FAILED_TO_INIT_BACKEND);
    }

    if (pDeviceID == NULL) {
        hr = mal_IMMDeviceEnumerator_GetDefaultAudioEndpoint(pDeviceEnumerator, (deviceType == mal_device_type_playback) ? mal_eRender : mal_eCapture, mal_eConsole, ppMMDevice);
    } else {
        hr = mal_IMMDeviceEnumerator_GetDevice(pDeviceEnumerator, pDeviceID->wasapi, ppMMDevice);
    }

    mal_IMMDeviceEnumerator_Release(pDeviceEnumerator);
    if (FAILED(hr)) {
        return mal_context_post_error(pContext, NULL, MAL_LOG_LEVEL_ERROR, "[WASAPI] Failed to retrieve IMMDevice.", MAL_FAILED_TO_OPEN_BACKEND_DEVICE);
    }

    return MAL_SUCCESS;
}

mal_result mal_context_get_device_info_from_MMDevice__wasapi(mal_context* pContext, mal_IMMDevice* pMMDevice, mal_share_mode shareMode, mal_bool32 onlySimpleInfo, mal_device_info* pInfo)
{
    mal_assert(pContext != NULL);
    mal_assert(pMMDevice != NULL);
    mal_assert(pInfo != NULL);

    // ID.
    LPWSTR id;
    HRESULT hr = mal_IMMDevice_GetId(pMMDevice, &id);
    if (SUCCEEDED(hr)) {
        size_t idlen = wcslen(id);
        if (idlen+1 > mal_countof(pInfo->id.wasapi)) {
            mal_CoTaskMemFree(pContext, id);
            mal_assert(MAL_FALSE);  // NOTE: If this is triggered, please report it. It means the format of the ID must haved change and is too long to fit in our fixed sized buffer.
            return MAL_ERROR;
        }

        mal_copy_memory(pInfo->id.wasapi, id, idlen * sizeof(wchar_t));
        pInfo->id.wasapi[idlen] = '\0';

        mal_CoTaskMemFree(pContext, id);
    }

    {
        mal_IPropertyStore *pProperties;
        hr = mal_IMMDevice_OpenPropertyStore(pMMDevice, STGM_READ, &pProperties);
        if (SUCCEEDED(hr)) {
            PROPVARIANT var;

            // Description / Friendly Name
            mal_PropVariantInit(&var);
            hr = mal_IPropertyStore_GetValue(pProperties, &MAL_PKEY_Device_FriendlyName, &var);
            if (SUCCEEDED(hr)) {
                WideCharToMultiByte(CP_UTF8, 0, var.pwszVal, -1, pInfo->name, sizeof(pInfo->name), 0, FALSE);
                mal_PropVariantClear(pContext, &var);
            }

            mal_IPropertyStore_Release(pProperties);
        }
    }

    // Format
    if (!onlySimpleInfo) {
        mal_IAudioClient* pAudioClient;
        hr = mal_IMMDevice_Activate(pMMDevice, &MAL_IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&pAudioClient);
        if (SUCCEEDED(hr)) {
            mal_result result = mal_context_get_device_info_from_IAudioClient__wasapi(pContext, pMMDevice, pAudioClient, shareMode, pInfo);
            
            mal_IAudioClient_Release(pAudioClient);
            return result;
        } else {
            return mal_context_post_error(pContext, NULL, MAL_LOG_LEVEL_ERROR, "[WASAPI] Failed to activate audio client for device info retrieval.", MAL_FAILED_TO_OPEN_BACKEND_DEVICE);
        }
    }

    return MAL_SUCCESS;
}

mal_result mal_context_enumerate_device_collection__wasapi(mal_context* pContext, mal_IMMDeviceCollection* pDeviceCollection, mal_device_type deviceType, mal_enum_devices_callback_proc callback, void* pUserData)
{
    mal_assert(pContext != NULL);
    mal_assert(callback != NULL);

    UINT deviceCount;
    HRESULT hr = mal_IMMDeviceCollection_GetCount(pDeviceCollection, &deviceCount);
    if (FAILED(hr)) {
        return mal_context_post_error(pContext, NULL, MAL_LOG_LEVEL_ERROR, "[WASAPI] Failed to get playback device count.", MAL_NO_DEVICE);
    }

    for (mal_uint32 iDevice = 0; iDevice < deviceCount; ++iDevice) {
        mal_device_info deviceInfo;
        mal_zero_object(&deviceInfo);

        mal_IMMDevice* pMMDevice;
        hr = mal_IMMDeviceCollection_Item(pDeviceCollection, iDevice, &pMMDevice);
        if (SUCCEEDED(hr)) {
            mal_result result = mal_context_get_device_info_from_MMDevice__wasapi(pContext, pMMDevice, mal_share_mode_shared, MAL_TRUE, &deviceInfo);   // MAL_TRUE = onlySimpleInfo.

            mal_IMMDevice_Release(pMMDevice);
            if (result == MAL_SUCCESS) {
                mal_bool32 cbResult = callback(pContext, deviceType, &deviceInfo, pUserData);
                if (cbResult == MAL_FALSE) {
                    break;
                }
            }
        }
    }

    return MAL_SUCCESS;
}
#endif

mal_result mal_context_enumerate_devices__wasapi(mal_context* pContext, mal_enum_devices_callback_proc callback, void* pUserData)
{
    mal_assert(pContext != NULL);
    mal_assert(callback != NULL);

    // Different enumeration for desktop and UWP.
#ifdef MAL_WIN32_DESKTOP
    // Desktop
    mal_IMMDeviceEnumerator* pDeviceEnumerator;
    HRESULT hr = mal_CoCreateInstance(pContext, MAL_CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, MAL_IID_IMMDeviceEnumerator, (void**)&pDeviceEnumerator);
    if (FAILED(hr)) {
        return mal_context_post_error(pContext, NULL, MAL_LOG_LEVEL_ERROR, "[WASAPI] Failed to create device enumerator.", MAL_FAILED_TO_OPEN_BACKEND_DEVICE);
    }

    mal_IMMDeviceCollection* pDeviceCollection;

    // Playback.
    hr = mal_IMMDeviceEnumerator_EnumAudioEndpoints(pDeviceEnumerator, mal_eRender, MAL_MM_DEVICE_STATE_ACTIVE, &pDeviceCollection);
    if (SUCCEEDED(hr)) {
        mal_context_enumerate_device_collection__wasapi(pContext, pDeviceCollection, mal_device_type_playback, callback, pUserData);
        mal_IMMDeviceCollection_Release(pDeviceCollection);
    }

    // Capture.
    hr = mal_IMMDeviceEnumerator_EnumAudioEndpoints(pDeviceEnumerator, mal_eCapture, MAL_MM_DEVICE_STATE_ACTIVE, &pDeviceCollection);
    if (SUCCEEDED(hr)) {
        mal_context_enumerate_device_collection__wasapi(pContext, pDeviceCollection, mal_device_type_capture, callback, pUserData);
        mal_IMMDeviceCollection_Release(pDeviceCollection);
    }

    mal_IMMDeviceEnumerator_Release(pDeviceEnumerator);
#else
    // UWP
    //
    // The MMDevice API is only supported on desktop applications. For now, while I'm still figuring out how to properly enumerate
    // over devices without using MMDevice, I'm restricting devices to defaults.
    //
    // Hint: DeviceInformation::FindAllAsync() with DeviceClass.AudioCapture/AudioRender. https://blogs.windows.com/buildingapps/2014/05/15/real-time-audio-in-windows-store-and-windows-phone-apps/
    if (callback) {
        mal_bool32 cbResult = MAL_TRUE;

        // Playback.
        if (cbResult) {
            mal_device_info deviceInfo;
            mal_zero_object(&deviceInfo);
            mal_strncpy_s(deviceInfo.name, sizeof(deviceInfo.name), MAL_DEFAULT_PLAYBACK_DEVICE_NAME, (size_t)-1);
            cbResult = callback(pContext, mal_device_type_playback, &deviceInfo, pUserData);
        }

        // Capture.
        if (cbResult) {
            mal_device_info deviceInfo;
            mal_zero_object(&deviceInfo);
            mal_strncpy_s(deviceInfo.name, sizeof(deviceInfo.name), MAL_DEFAULT_CAPTURE_DEVICE_NAME, (size_t)-1);
            cbResult = callback(pContext, mal_device_type_capture, &deviceInfo, pUserData);
        }
    }
#endif

    return MAL_SUCCESS;
}

mal_result mal_context_get_device_info__wasapi(mal_context* pContext, mal_device_type deviceType, const mal_device_id* pDeviceID, mal_share_mode shareMode, mal_device_info* pDeviceInfo)
{
#ifdef MAL_WIN32_DESKTOP
    mal_IMMDevice* pMMDevice = NULL;
    mal_result result = mal_context_get_MMDevice__wasapi(pContext, deviceType, pDeviceID, &pMMDevice);
    if (result != MAL_SUCCESS) {
        return result;
    }

    result = mal_context_get_device_info_from_MMDevice__wasapi(pContext, pMMDevice, shareMode, MAL_FALSE, pDeviceInfo);   // MAL_FALSE = !onlySimpleInfo.

    mal_IMMDevice_Release(pMMDevice);
    return result;
#else
    // UWP currently only uses default devices.
    if (deviceType == mal_device_type_playback) {
        mal_strncpy_s(pDeviceInfo->name, sizeof(pDeviceInfo->name), MAL_DEFAULT_PLAYBACK_DEVICE_NAME, (size_t)-1);
    } else {
        mal_strncpy_s(pDeviceInfo->name, sizeof(pDeviceInfo->name), MAL_DEFAULT_CAPTURE_DEVICE_NAME, (size_t)-1);
    }

    // Not currently supporting exclusive mode on UWP.
    if (shareMode == mal_share_mode_exclusive) {
        return MAL_ERROR;
    }

    mal_IAudioClient* pAudioClient;
    mal_result result = mal_context_get_IAudioClient_UWP__wasapi(pContext, deviceType, pDeviceID, &pAudioClient, NULL);
    if (result != MAL_SUCCESS) {
        return result;
    }

    result = mal_context_get_device_info_from_IAudioClient__wasapi(pContext, NULL, pAudioClient, shareMode, pDeviceInfo);

    mal_IAudioClient_Release(pAudioClient);
    return result;
#endif
}

void mal_device_uninit__wasapi(mal_device* pDevice)
{
    mal_assert(pDevice != NULL);

#ifdef MAL_WIN32_DESKTOP
    if (pDevice->wasapi.pDeviceEnumerator) {
        ((mal_IMMDeviceEnumerator*)pDevice->wasapi.pDeviceEnumerator)->lpVtbl->UnregisterEndpointNotificationCallback((mal_IMMDeviceEnumerator*)pDevice->wasapi.pDeviceEnumerator, &pDevice->wasapi.notificationClient);
        mal_IMMDeviceEnumerator_Release((mal_IMMDeviceEnumerator*)pDevice->wasapi.pDeviceEnumerator);
    }
#endif

    if (pDevice->wasapi.pRenderClient) {
        mal_IAudioRenderClient_Release((mal_IAudioRenderClient*)pDevice->wasapi.pRenderClient);
    }
    if (pDevice->wasapi.pCaptureClient) {
        mal_IAudioCaptureClient_Release((mal_IAudioCaptureClient*)pDevice->wasapi.pCaptureClient);
    }
    if (pDevice->wasapi.pAudioClient) {
        mal_IAudioClient_Release((mal_IAudioClient*)pDevice->wasapi.pAudioClient);
    }

    if (pDevice->wasapi.hEvent) {
        CloseHandle(pDevice->wasapi.hEvent);
    }
    if (pDevice->wasapi.hBreakEvent) {
        CloseHandle(pDevice->wasapi.hBreakEvent);
    }
}

typedef struct
{
    // Input.
    mal_format formatIn;
    mal_uint32 channelsIn;
    mal_uint32 sampleRateIn;
    mal_channel channelMapIn[MAL_MAX_CHANNELS];
    mal_uint32 bufferSizeInFramesIn;
    mal_uint32 bufferSizeInMillisecondsIn;
    mal_uint32 periodsIn;
    mal_bool32 usingDefaultFormat;
    mal_bool32 usingDefaultChannels;
    mal_bool32 usingDefaultSampleRate;
    mal_bool32 usingDefaultChannelMap;
    mal_share_mode shareMode;

    // Output.
    mal_IAudioClient* pAudioClient;
    mal_IAudioRenderClient* pRenderClient;
    mal_IAudioCaptureClient* pCaptureClient;
    mal_format formatOut;
    mal_uint32 channelsOut;
    mal_uint32 sampleRateOut;
    mal_channel channelMapOut[MAL_MAX_CHANNELS];
    mal_uint32 bufferSizeInFramesOut;
    mal_uint32 periodsOut;
    mal_bool32 exclusiveMode;
    char deviceName[256];
} mal_device_init_internal_data__wasapi;

mal_result mal_device_init_internal__wasapi(mal_context* pContext, mal_device_type type, const mal_device_id* pDeviceID, mal_device_init_internal_data__wasapi* pData)
{
    (void)pContext;

    mal_assert(pContext != NULL);
    mal_assert(pData != NULL);

    pData->pAudioClient = NULL;
    pData->pRenderClient = NULL;
    pData->pCaptureClient = NULL;
    

    HRESULT hr;
    mal_result result = MAL_SUCCESS;
    const char* errorMsg = "";
    MAL_AUDCLNT_SHAREMODE shareMode = MAL_AUDCLNT_SHAREMODE_SHARED;
    MAL_REFERENCE_TIME bufferDurationInMicroseconds;
    mal_bool32 wasInitializedUsingIAudioClient3 = MAL_FALSE;
    WAVEFORMATEXTENSIBLE wf;

#ifdef MAL_WIN32_DESKTOP
    mal_IMMDevice* pMMDevice = NULL;
    result = mal_context_get_MMDevice__wasapi(pContext, type, pDeviceID, &pMMDevice);
    if (result != MAL_SUCCESS) {
        goto done;
    }

    hr = mal_IMMDevice_Activate(pMMDevice, &MAL_IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&pData->pAudioClient);
    if (FAILED(hr)) {
        errorMsg = "[WASAPI] Failed to activate device.", result = MAL_FAILED_TO_OPEN_BACKEND_DEVICE;
        goto done;
    }
#else
    mal_IUnknown* pActivatedInterface = NULL;
    result = mal_context_get_IAudioClient_UWP__wasapi(pContext, type, pDeviceID, &pData->pAudioClient, &pActivatedInterface);
    if (result != MAL_SUCCESS) {
        goto done;
    }
#endif

    // Try enabling hardware offloading.
    mal_IAudioClient2* pAudioClient2;
    hr = mal_IAudioClient_QueryInterface(pData->pAudioClient, &MAL_IID_IAudioClient2, (void**)&pAudioClient2);
    if (SUCCEEDED(hr)) {
        BOOL isHardwareOffloadingSupported = 0;
        hr = mal_IAudioClient2_IsOffloadCapable(pAudioClient2, MAL_AudioCategory_Other, &isHardwareOffloadingSupported);
        if (SUCCEEDED(hr) && isHardwareOffloadingSupported) {
            mal_AudioClientProperties clientProperties;
            mal_zero_object(&clientProperties);
            clientProperties.cbSize = sizeof(clientProperties);
            clientProperties.bIsOffload = 1;
            clientProperties.eCategory = MAL_AudioCategory_Other;
            mal_IAudioClient2_SetClientProperties(pAudioClient2, &clientProperties);
        }
    }


    // Here is where we try to determine the best format to use with the device. If the client if wanting exclusive mode, first try finding the best format for that. If this fails, fall back to shared mode.
    result = MAL_FORMAT_NOT_SUPPORTED;
    if (pData->shareMode == mal_share_mode_exclusive) {
    #ifdef MAL_WIN32_DESKTOP
        // In exclusive mode on desktop we always use the backend's native format.
        mal_IPropertyStore* pStore = NULL;
        hr = mal_IMMDevice_OpenPropertyStore(pMMDevice, STGM_READ, &pStore);
        if (SUCCEEDED(hr)) {
            PROPVARIANT prop;
            mal_PropVariantInit(&prop);
            hr = mal_IPropertyStore_GetValue(pStore, &MAL_PKEY_AudioEngine_DeviceFormat, &prop);
            if (SUCCEEDED(hr)) {
                WAVEFORMATEX* pActualFormat = (WAVEFORMATEX*)prop.blob.pBlobData;
                hr = mal_IAudioClient_IsFormatSupported((mal_IAudioClient*)pData->pAudioClient, MAL_AUDCLNT_SHAREMODE_EXCLUSIVE, pActualFormat, NULL);
                if (SUCCEEDED(hr)) {
                    mal_copy_memory(&wf, pActualFormat, sizeof(WAVEFORMATEXTENSIBLE));
                }

                mal_PropVariantClear(pContext, &prop);
            }

            mal_IPropertyStore_Release(pStore);
        }
    #else
        // I do not know how to query the device's native format on UWP so for now I'm just disabling support for
        // exclusive mode. The alternative is to enumerate over different formats and check IsFormatSupported()
        // until you find one that works.
        //
        // TODO: Add support for exclusive mode to UWP.
        hr = S_FALSE;
    #endif

        if (hr == S_OK) {
            shareMode = MAL_AUDCLNT_SHAREMODE_EXCLUSIVE;
            result = MAL_SUCCESS;
        }
    }

    // Fall back to shared mode if necessary.
    if (result != MAL_SUCCESS) {
        // In shared mode we are always using the format reported by the operating system.
        WAVEFORMATEXTENSIBLE* pNativeFormat = NULL;
        hr = mal_IAudioClient_GetMixFormat((mal_IAudioClient*)pData->pAudioClient, (WAVEFORMATEX**)&pNativeFormat);
        if (hr != S_OK) {
            result = MAL_FORMAT_NOT_SUPPORTED;
        } else {
            mal_copy_memory(&wf, pNativeFormat, sizeof(wf));
            result = MAL_SUCCESS;
        }

        mal_CoTaskMemFree(pContext, pNativeFormat);

        shareMode = MAL_AUDCLNT_SHAREMODE_SHARED;
    }

    // Return an error if we still haven't found a format.
    if (result != MAL_SUCCESS) {
        errorMsg = "[WASAPI] Failed to find best device mix format.", result = MAL_FORMAT_NOT_SUPPORTED;
        goto done;
    }

    pData->formatOut = mal_format_from_WAVEFORMATEX((WAVEFORMATEX*)&wf);
    pData->channelsOut = wf.Format.nChannels;
    pData->sampleRateOut = wf.Format.nSamplesPerSec;

    // Get the internal channel map based on the channel mask.
    mal_channel_mask_to_channel_map__win32(wf.dwChannelMask, pData->channelsOut, pData->channelMapOut);

    // If we're using a default buffer size we need to calculate it based on the efficiency of the system.
    pData->periodsOut = pData->periodsIn;
    pData->bufferSizeInFramesOut = pData->bufferSizeInFramesIn;
    if (pData->bufferSizeInFramesOut == 0) {
        pData->bufferSizeInFramesOut = mal_calculate_buffer_size_in_frames_from_milliseconds(pData->bufferSizeInMillisecondsIn, pData->sampleRateOut);
    }

    bufferDurationInMicroseconds = ((mal_uint64)pData->bufferSizeInFramesOut * 1000 * 1000) / pData->sampleRateOut;


    // Slightly different initialization for shared and exclusive modes. We try exclusive mode first, and if it fails, fall back to shared mode.
    if (shareMode == MAL_AUDCLNT_SHAREMODE_EXCLUSIVE) {
        // Exclusive.
        MAL_REFERENCE_TIME bufferDuration = bufferDurationInMicroseconds*10;

        // If the periodicy is too small, Initialize() will fail with AUDCLNT_E_INVALID_DEVICE_PERIOD. In this case we should just keep increasing
        // it and trying it again.
        hr = E_FAIL;
        for (;;) {
            hr = mal_IAudioClient_Initialize((mal_IAudioClient*)pData->pAudioClient, shareMode, MAL_AUDCLNT_STREAMFLAGS_EVENTCALLBACK, bufferDuration, bufferDuration, (WAVEFORMATEX*)&wf, NULL);
            if (hr == MAL_AUDCLNT_E_INVALID_DEVICE_PERIOD) {
                if (bufferDuration > 500*10000) {
                    break;
                } else {
                    if (bufferDuration == 0) {  // <-- Just a sanity check to prevent an infinit loop. Should never happen, but it makes me feel better.
                        break;
                    }

                    bufferDuration = bufferDuration * 2;
                    continue;
                }
            } else {
                break;
            }
        }
        
        if (hr == MAL_AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED) {
            UINT bufferSizeInFrames;
            hr = mal_IAudioClient_GetBufferSize((mal_IAudioClient*)pData->pAudioClient, &bufferSizeInFrames);
            if (SUCCEEDED(hr)) {
                bufferDuration = (MAL_REFERENCE_TIME)((10000.0 * 1000 / wf.Format.nSamplesPerSec * bufferSizeInFrames) + 0.5);

                // Unfortunately we need to release and re-acquire the audio client according to MSDN. Seems silly - why not just call IAudioClient_Initialize() again?!
                mal_IAudioClient_Release((mal_IAudioClient*)pData->pAudioClient);

            #ifdef MAL_WIN32_DESKTOP
                hr = mal_IMMDevice_Activate(pMMDevice, &MAL_IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&pData->pAudioClient);
            #else
                hr = mal_IUnknown_QueryInterface(pActivatedInterface, &MAL_IID_IAudioClient, (void**)&pData->pAudioClient);
            #endif

                if (SUCCEEDED(hr)) {
                    hr = mal_IAudioClient_Initialize((mal_IAudioClient*)pData->pAudioClient, shareMode, MAL_AUDCLNT_STREAMFLAGS_EVENTCALLBACK, bufferDuration, bufferDuration, (WAVEFORMATEX*)&wf, NULL);
                }
            }
        }

        if (FAILED(hr)) {
            // Failed to initialize in exclusive mode. We don't return an error here, but instead fall back to shared mode.
            shareMode = MAL_AUDCLNT_SHAREMODE_SHARED;
        }
    }

    if (shareMode == MAL_AUDCLNT_SHAREMODE_SHARED) {
        // Shared.

        // Low latency shared mode via IAudioClient3.
        mal_IAudioClient3* pAudioClient3 = NULL;
        hr = mal_IAudioClient_QueryInterface(pData->pAudioClient, &MAL_IID_IAudioClient3, (void**)&pAudioClient3);
        if (SUCCEEDED(hr)) {
            UINT32 defaultPeriodInFrames;
            UINT32 fundamentalPeriodInFrames;
            UINT32 minPeriodInFrames;
            UINT32 maxPeriodInFrames;
            hr = mal_IAudioClient3_GetSharedModeEnginePeriod(pAudioClient3, (WAVEFORMATEX*)&wf, &defaultPeriodInFrames, &fundamentalPeriodInFrames, &minPeriodInFrames, &maxPeriodInFrames);
            if (SUCCEEDED(hr)) {
                UINT32 desiredPeriodInFrames = pData->bufferSizeInFramesOut / pData->periodsOut;
                
                // Make sure the period size is a multiple of fundamentalPeriodInFrames.
                desiredPeriodInFrames = desiredPeriodInFrames / fundamentalPeriodInFrames;
                desiredPeriodInFrames = desiredPeriodInFrames * fundamentalPeriodInFrames;

                // The period needs to be clamped between minPeriodInFrames and maxPeriodInFrames.
                desiredPeriodInFrames = mal_clamp(desiredPeriodInFrames, minPeriodInFrames, maxPeriodInFrames);
                
                hr = mal_IAudioClient3_InitializeSharedAudioStream(pAudioClient3, MAL_AUDCLNT_STREAMFLAGS_EVENTCALLBACK, desiredPeriodInFrames, (WAVEFORMATEX*)&wf, NULL);
                if (SUCCEEDED(hr)) {
                    wasInitializedUsingIAudioClient3 = MAL_TRUE;
                    pData->bufferSizeInFramesOut = desiredPeriodInFrames * pData->periodsOut;
                }
            }

            mal_IAudioClient3_Release(pAudioClient3);
            pAudioClient3 = NULL;
        }

        // If we don't have an IAudioClient3 then we need to use the normal initialization routine.
        if (!wasInitializedUsingIAudioClient3) {
            MAL_REFERENCE_TIME bufferDuration = bufferDurationInMicroseconds*10;
            hr = mal_IAudioClient_Initialize((mal_IAudioClient*)pData->pAudioClient, shareMode, MAL_AUDCLNT_STREAMFLAGS_EVENTCALLBACK, bufferDuration, 0, (WAVEFORMATEX*)&wf, NULL);
            if (FAILED(hr)) {
                if (hr == E_ACCESSDENIED) {
                    errorMsg = "[WASAPI] Failed to initialize device. Access denied.", result = MAL_ACCESS_DENIED;
                } else if (hr == MAL_AUDCLNT_E_DEVICE_IN_USE) {
                    errorMsg = "[WASAPI] Failed to initialize device. Device in use.", result = MAL_DEVICE_BUSY;
                } else {
                    errorMsg = "[WASAPI] Failed to initialize device.", result = MAL_FAILED_TO_OPEN_BACKEND_DEVICE;
                }

                goto done;
            }
        }
    }

    if (!wasInitializedUsingIAudioClient3) {
        hr = mal_IAudioClient_GetBufferSize((mal_IAudioClient*)pData->pAudioClient, &pData->bufferSizeInFramesOut);
        if (FAILED(hr)) {
            errorMsg = "[WASAPI] Failed to get audio client's actual buffer size.", result = MAL_FAILED_TO_OPEN_BACKEND_DEVICE;
            goto done;
        }
    }

    if (type == mal_device_type_playback) {
        hr = mal_IAudioClient_GetService((mal_IAudioClient*)pData->pAudioClient, &MAL_IID_IAudioRenderClient, (void**)&pData->pRenderClient);
    } else {
        hr = mal_IAudioClient_GetService((mal_IAudioClient*)pData->pAudioClient, &MAL_IID_IAudioCaptureClient, (void**)&pData->pCaptureClient);
    }

    if (FAILED(hr)) {
        errorMsg = "[WASAPI] Failed to get audio client service.", result = MAL_API_NOT_FOUND;
        goto done;
    }


    if (shareMode == MAL_AUDCLNT_SHAREMODE_SHARED) {
        pData->exclusiveMode = MAL_FALSE;
    } else /*if (shareMode == MAL_AUDCLNT_SHAREMODE_EXCLUSIVE)*/ {
        pData->exclusiveMode = MAL_TRUE;
    }


    // Grab the name of the device.
#ifdef MAL_WIN32_DESKTOP
    mal_IPropertyStore *pProperties;
    hr = mal_IMMDevice_OpenPropertyStore(pMMDevice, STGM_READ, &pProperties);
    if (SUCCEEDED(hr)) {
        PROPVARIANT varName;
        mal_PropVariantInit(&varName);
        hr = mal_IPropertyStore_GetValue(pProperties, &MAL_PKEY_Device_FriendlyName, &varName);
        if (SUCCEEDED(hr)) {
            WideCharToMultiByte(CP_UTF8, 0, varName.pwszVal, -1, pData->deviceName, sizeof(pData->deviceName), 0, FALSE);
            mal_PropVariantClear(pContext, &varName);
        }

        mal_IPropertyStore_Release(pProperties);
    }
#endif

done:
    // Clean up.
#ifdef MAL_WIN32_DESKTOP
    if (pMMDevice != NULL) {
        mal_IMMDevice_Release(pMMDevice);
    }
#else
    if (pActivatedInterface != NULL) {
        mal_IUnknown_Release(pActivatedInterface);
    }
#endif

    if (result != MAL_SUCCESS) {
        if (pData->pRenderClient) {
            mal_IAudioRenderClient_Release((mal_IAudioRenderClient*)pData->pRenderClient);
            pData->pRenderClient = NULL;
        }
        if (pData->pCaptureClient) {
            mal_IAudioCaptureClient_Release((mal_IAudioCaptureClient*)pData->pCaptureClient);
            pData->pCaptureClient = NULL;
        }
        if (pData->pAudioClient) {
            mal_IAudioClient_Release((mal_IAudioClient*)pData->pAudioClient);
            pData->pAudioClient = NULL;
        }

        return mal_context_post_error(pContext, NULL, MAL_LOG_LEVEL_ERROR, errorMsg, result);
    } else {
        return MAL_SUCCESS;
    }
}

mal_result mal_device_reinit__wasapi(mal_device* pDevice)
{
    mal_device_init_internal_data__wasapi data;
    data.formatIn = pDevice->format;
    data.channelsIn = pDevice->channels;
    data.sampleRateIn = pDevice->sampleRate;
    mal_copy_memory(data.channelMapIn, pDevice->channelMap, sizeof(pDevice->channelMap));
    data.bufferSizeInFramesIn = pDevice->bufferSizeInFrames;
    data.bufferSizeInMillisecondsIn = pDevice->bufferSizeInMilliseconds;
    data.periodsIn = pDevice->periods;
    data.usingDefaultFormat = pDevice->usingDefaultFormat;
    data.usingDefaultChannels = pDevice->usingDefaultChannels;
    data.usingDefaultSampleRate = pDevice->usingDefaultSampleRate;
    data.usingDefaultChannelMap = pDevice->usingDefaultChannelMap;
    data.shareMode = pDevice->initConfig.shareMode;
    mal_result result = mal_device_init_internal__wasapi(pDevice->pContext, pDevice->type, NULL, &data);
    if (result != MAL_SUCCESS) {
        return result;
    }

    // At this point we have some new objects ready to go. We need to uninitialize the previous ones and then set the new ones.
    if (pDevice->wasapi.pRenderClient) {
        mal_IAudioRenderClient_Release((mal_IAudioRenderClient*)pDevice->wasapi.pRenderClient);
        pDevice->wasapi.pRenderClient = NULL;
    }
    if (pDevice->wasapi.pCaptureClient) {
        mal_IAudioCaptureClient_Release((mal_IAudioCaptureClient*)pDevice->wasapi.pCaptureClient);
        pDevice->wasapi.pCaptureClient = NULL;
    }
    if (pDevice->wasapi.pAudioClient) {
        mal_IAudioClient_Release((mal_IAudioClient*)pDevice->wasapi.pAudioClient);
        pDevice->wasapi.pAudioClient = NULL;
    }

    pDevice->wasapi.pAudioClient = data.pAudioClient;
    pDevice->wasapi.pRenderClient = data.pRenderClient;
    pDevice->wasapi.pCaptureClient = data.pCaptureClient;
    
    pDevice->internalFormat = data.formatOut;
    pDevice->internalChannels = data.channelsOut;
    pDevice->internalSampleRate = data.sampleRateOut;
    mal_copy_memory(pDevice->internalChannelMap, data.channelMapOut, sizeof(data.channelMapOut));
    pDevice->bufferSizeInFrames = data.bufferSizeInFramesOut;
    pDevice->periods = data.periodsOut;
    pDevice->exclusiveMode = data.exclusiveMode;
    mal_strcpy_s(pDevice->name, sizeof(pDevice->name), data.deviceName);

    mal_IAudioClient_SetEventHandle((mal_IAudioClient*)pDevice->wasapi.pAudioClient, pDevice->wasapi.hEvent);

    return MAL_SUCCESS;
}

mal_result mal_device_init__wasapi(mal_context* pContext, mal_device_type type, const mal_device_id* pDeviceID, const mal_device_config* pConfig, mal_device* pDevice)
{
    (void)pContext;
    (void)pConfig;

    mal_assert(pDevice != NULL);
    mal_zero_object(&pDevice->wasapi);

    mal_result result = MAL_SUCCESS;
    const char* errorMsg = "";

    mal_device_init_internal_data__wasapi data;
    data.formatIn = pDevice->format;
    data.channelsIn = pDevice->channels;
    data.sampleRateIn = pDevice->sampleRate;
    mal_copy_memory(data.channelMapIn, pDevice->channelMap, sizeof(pDevice->channelMap));
    data.bufferSizeInFramesIn = pDevice->bufferSizeInFrames;
    data.bufferSizeInMillisecondsIn = pDevice->bufferSizeInMilliseconds;
    data.periodsIn = pDevice->periods;
    data.usingDefaultFormat = pDevice->usingDefaultFormat;
    data.usingDefaultChannels = pDevice->usingDefaultChannels;
    data.usingDefaultSampleRate = pDevice->usingDefaultSampleRate;
    data.usingDefaultChannelMap = pDevice->usingDefaultChannelMap;
    data.shareMode = pDevice->initConfig.shareMode;
    result = mal_device_init_internal__wasapi(pDevice->pContext, type, pDeviceID, &data);
    if (result != MAL_SUCCESS) {
        return result;
    }

    pDevice->wasapi.pAudioClient = data.pAudioClient;
    pDevice->wasapi.pRenderClient = data.pRenderClient;
    pDevice->wasapi.pCaptureClient = data.pCaptureClient;
    
    pDevice->internalFormat = data.formatOut;
    pDevice->internalChannels = data.channelsOut;
    pDevice->internalSampleRate = data.sampleRateOut;
    mal_copy_memory(pDevice->internalChannelMap, data.channelMapOut, sizeof(data.channelMapOut));
    pDevice->bufferSizeInFrames = data.bufferSizeInFramesOut;
    pDevice->periods = data.periodsOut;
    pDevice->exclusiveMode = data.exclusiveMode;
    mal_strcpy_s(pDevice->name, sizeof(pDevice->name), data.deviceName);



    // We need to get notifications of when the default device changes. We do this through a device enumerator by
    // registering a IMMNotificationClient with it. We only care about this if it's the default device.
#ifdef MAL_WIN32_DESKTOP
    mal_IMMDeviceEnumerator* pDeviceEnumerator;
    HRESULT hr = mal_CoCreateInstance(pContext, MAL_CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, MAL_IID_IMMDeviceEnumerator, (void**)&pDeviceEnumerator);
    if (FAILED(hr)) {
        errorMsg = "[WASAPI] Failed to create device enumerator.", result = MAL_FAILED_TO_OPEN_BACKEND_DEVICE;
        goto done;
    }

    pDevice->wasapi.notificationClient.lpVtbl  = (void*)&g_malNotificationCientVtbl;
    pDevice->wasapi.notificationClient.counter = 1;
    pDevice->wasapi.notificationClient.pDevice = pDevice;

    hr = pDeviceEnumerator->lpVtbl->RegisterEndpointNotificationCallback(pDeviceEnumerator, &pDevice->wasapi.notificationClient);
    if (SUCCEEDED(hr)) {
        pDevice->wasapi.pDeviceEnumerator = (mal_ptr)pDeviceEnumerator;
    } else {
        // Not the end of the world if we fail to register the notification callback. We just won't support automatic stream routing.
        mal_IMMDeviceEnumerator_Release(pDeviceEnumerator);
    }
#endif


    // We need to create and set the event for event-driven mode. This event is signalled whenever a new chunk of audio
    // data needs to be written or read from the device.
    pDevice->wasapi.hEvent = CreateEventA(NULL, FALSE, FALSE, NULL);
    if (pDevice->wasapi.hEvent == NULL) {
        errorMsg = "[WASAPI] Failed to create main event for main loop.", result = MAL_FAILED_TO_CREATE_EVENT;
        goto done;
    }

    mal_IAudioClient_SetEventHandle((mal_IAudioClient*)pDevice->wasapi.pAudioClient, pDevice->wasapi.hEvent);


    // When the device is playing the worker thread will be waiting on a bunch of notification events. To return from
    // this wait state we need to signal a special event.
    pDevice->wasapi.hBreakEvent = CreateEventA(NULL, FALSE, FALSE, NULL);
    if (pDevice->wasapi.hBreakEvent == NULL) {
        errorMsg = "[WASAPI] Failed to create break event for main loop break notification.", result = MAL_FAILED_TO_CREATE_EVENT;
        goto done;
    }

    result = MAL_SUCCESS;

done:
    // Clean up.
    if (result != MAL_SUCCESS) {
        mal_device_uninit__wasapi(pDevice);
        return mal_post_error(pDevice, MAL_LOG_LEVEL_ERROR, errorMsg, result);
    } else {
        return MAL_SUCCESS;
    }
}

mal_result mal_device__start_backend__wasapi(mal_device* pDevice)
{
    mal_assert(pDevice != NULL);

    // Playback devices need to have an initial chunk of data loaded.
    if (pDevice->type == mal_device_type_playback) {
        BYTE* pData;
        HRESULT hr = mal_IAudioRenderClient_GetBuffer((mal_IAudioRenderClient*)pDevice->wasapi.pRenderClient, pDevice->bufferSizeInFrames, &pData);
        if (FAILED(hr)) {
            return mal_post_error(pDevice, MAL_LOG_LEVEL_ERROR, "[WASAPI] Failed to retrieve buffer from internal playback device.", MAL_FAILED_TO_MAP_DEVICE_BUFFER);
        }

        mal_device__read_frames_from_client(pDevice, pDevice->bufferSizeInFrames, pData);

        hr = mal_IAudioRenderClient_ReleaseBuffer((mal_IAudioRenderClient*)pDevice->wasapi.pRenderClient, pDevice->bufferSizeInFrames, 0);
        if (FAILED(hr)) {
            return mal_post_error(pDevice, MAL_LOG_LEVEL_ERROR, "[WASAPI] Failed to release internal buffer for playback device.", MAL_FAILED_TO_UNMAP_DEVICE_BUFFER);
        }
    }

    HRESULT hr = mal_IAudioClient_Start((mal_IAudioClient*)pDevice->wasapi.pAudioClient);
    if (FAILED(hr)) {
        return mal_post_error(pDevice, MAL_LOG_LEVEL_ERROR, "[WASAPI] Failed to start internal device.", MAL_FAILED_TO_START_BACKEND_DEVICE);
    }

    return MAL_SUCCESS;
}

mal_result mal_device__stop_backend__wasapi(mal_device* pDevice)
{
    mal_assert(pDevice != NULL);

    if (pDevice->wasapi.pAudioClient == NULL) {
        return MAL_DEVICE_NOT_INITIALIZED;
    }

    HRESULT hr = mal_IAudioClient_Stop((mal_IAudioClient*)pDevice->wasapi.pAudioClient);
    if (FAILED(hr)) {
        return mal_post_error(pDevice, MAL_LOG_LEVEL_ERROR, "[WASAPI] Failed to stop internal device.", MAL_FAILED_TO_STOP_BACKEND_DEVICE);
    }

    // The client needs to be reset or else we won't be able to resume it again.
    hr = mal_IAudioClient_Reset((mal_IAudioClient*)pDevice->wasapi.pAudioClient);
    if (FAILED(hr)) {
        return mal_post_error(pDevice, MAL_LOG_LEVEL_ERROR, "[WASAPI] Failed to reset internal device.", MAL_FAILED_TO_STOP_BACKEND_DEVICE);
    }

    return MAL_SUCCESS;
}

mal_result mal_device__break_main_loop__wasapi(mal_device* pDevice)
{
    mal_assert(pDevice != NULL);

    // The main loop will be waiting on a bunch of events via the WaitForMultipleObjects() API. One of those events
    // is a special event we use for forcing that function to return.
    pDevice->wasapi.breakFromMainLoop = MAL_TRUE;
    SetEvent(pDevice->wasapi.hBreakEvent);
    return MAL_SUCCESS;
}

mal_result mal_device__get_available_frames__wasapi(mal_device* pDevice, mal_uint32* pFrameCount)
{
    mal_assert(pDevice != NULL);
    mal_assert(pFrameCount != NULL);
    
    *pFrameCount = 0;

    mal_uint32 paddingFramesCount;
    HRESULT hr = mal_IAudioClient_GetCurrentPadding((mal_IAudioClient*)pDevice->wasapi.pAudioClient, &paddingFramesCount);
    if (FAILED(hr)) {
        return MAL_DEVICE_UNAVAILABLE;
    }

    // Slightly different rules for exclusive and shared modes.
    if (pDevice->exclusiveMode) {
        *pFrameCount = paddingFramesCount;
    } else {
        if (pDevice->type == mal_device_type_playback) {
            *pFrameCount = pDevice->bufferSizeInFrames - paddingFramesCount;
        } else {
            *pFrameCount = paddingFramesCount;
        }
    }

    return MAL_SUCCESS;
}

mal_result mal_device__wait_for_frames__wasapi(mal_device* pDevice, mal_uint32* pFrameCount)
{
    mal_assert(pDevice != NULL);

    mal_result result;

    while (!pDevice->wasapi.breakFromMainLoop) {
        // Wait for a buffer to become available or for the stop event to be signalled.
        HANDLE hEvents[2];
        hEvents[0] = (HANDLE)pDevice->wasapi.hEvent;
        hEvents[1] = (HANDLE)pDevice->wasapi.hBreakEvent;
        if (WaitForMultipleObjects(mal_countof(hEvents), hEvents, FALSE, INFINITE) == WAIT_FAILED) {
            break;
        }

        // Break from the main loop if the device isn't started anymore. Likely what's happened is the application
        // has requested that the device be stopped.
        if (!mal_device_is_started(pDevice)) {
            break;
        }

        // Make sure we break from the main loop if requested from an external factor.
        if (pDevice->wasapi.breakFromMainLoop) {
            break;
        }

        // We may want to reinitialize the device. Only do this if this device is the default.
        mal_bool32 needDeviceReinit = MAL_FALSE;

        mal_bool32 hasDefaultDeviceChanged = pDevice->wasapi.hasDefaultDeviceChanged;
        if (hasDefaultDeviceChanged && pDevice->isDefaultDevice) {
            needDeviceReinit = MAL_TRUE;
        }

        if (!needDeviceReinit) {
            result = mal_device__get_available_frames__wasapi(pDevice, pFrameCount);
            if (result != MAL_SUCCESS) {
                if (!pDevice->exclusiveMode) {
                    needDeviceReinit = MAL_TRUE;
                } else {
                    return result;
                }
            }
        }


        mal_atomic_exchange_32(&pDevice->wasapi.hasDefaultDeviceChanged, MAL_FALSE);

        // Here is where the device is re-initialized if required.
        if (needDeviceReinit) {
            #ifdef MAL_DEBUG_OUTPUT
                printf("=== CHANGING DEVICE ===\n");
            #endif

            if (pDevice->pContext->onDeviceReinit) {
                mal_result reinitResult = pDevice->pContext->onDeviceReinit(pDevice);
                if (reinitResult != MAL_SUCCESS) {
                    return reinitResult;
                }

                mal_device__post_init_setup(pDevice);

                // Start playing the device again, and then continue the loop from the top.
                if (mal_device__get_state(pDevice) == MAL_STATE_STARTED) {
                    if (pDevice->pContext->onDeviceStart) {
                        pDevice->pContext->onDeviceStart(pDevice);
                    }
                    continue;
                }
            }
        }
        

        if (*pFrameCount > 0) {
            return MAL_SUCCESS;
        }
    }

    // We'll get here if the loop was terminated. Just return whatever's available.
    return mal_device__get_available_frames__wasapi(pDevice, pFrameCount);
}

mal_result mal_device__main_loop__wasapi(mal_device* pDevice)
{
    mal_assert(pDevice != NULL);

    // Make sure the break event is not signaled to ensure we don't end up immediately returning from WaitForMultipleObjects().
    ResetEvent(pDevice->wasapi.hBreakEvent);

    pDevice->wasapi.breakFromMainLoop = MAL_FALSE;
    while (!pDevice->wasapi.breakFromMainLoop) {
        mal_uint32 framesAvailable;
        mal_result result = mal_device__wait_for_frames__wasapi(pDevice, &framesAvailable);
        if (result != MAL_SUCCESS) {
            return result;
        }

        if (framesAvailable == 0) {
            continue;
        }

        // If it's a playback device, don't bother grabbing more data if the device is being stopped.
        if (pDevice->wasapi.breakFromMainLoop && pDevice->type == mal_device_type_playback) {
            return MAL_SUCCESS;
        }

        if (pDevice->type == mal_device_type_playback) {
            BYTE* pData;
            HRESULT hr = mal_IAudioRenderClient_GetBuffer((mal_IAudioRenderClient*)pDevice->wasapi.pRenderClient, framesAvailable, &pData);
            if (FAILED(hr)) {
                return mal_post_error(pDevice, MAL_LOG_LEVEL_ERROR, "[WASAPI] Failed to retrieve internal buffer from playback device in preparation for sending new data to the device.", MAL_FAILED_TO_MAP_DEVICE_BUFFER);
            }

            mal_device__read_frames_from_client(pDevice, framesAvailable, pData);

            hr = mal_IAudioRenderClient_ReleaseBuffer((mal_IAudioRenderClient*)pDevice->wasapi.pRenderClient, framesAvailable, 0);
            if (FAILED(hr)) {
                return mal_post_error(pDevice, MAL_LOG_LEVEL_ERROR, "[WASAPI] Failed to release internal buffer from playback device in preparation for sending new data to the device.", MAL_FAILED_TO_UNMAP_DEVICE_BUFFER);
            }
        } else {
            mal_uint32 framesRemaining = framesAvailable;
            while (framesRemaining > 0) {
                BYTE* pData;
                mal_uint32 framesToSend;
                DWORD flags;
                HRESULT hr = mal_IAudioCaptureClient_GetBuffer((mal_IAudioCaptureClient*)pDevice->wasapi.pCaptureClient, &pData, &framesToSend, &flags, NULL, NULL);
                if (FAILED(hr)) {
                    mal_post_error(pDevice, MAL_LOG_LEVEL_ERROR, "[WASAPI] WARNING: Failed to retrieve internal buffer from capture device in preparation for sending new data to the client.", MAL_FAILED_TO_MAP_DEVICE_BUFFER);
                    break;
                }

                if (hr != MAL_AUDCLNT_S_BUFFER_EMPTY) {
                    mal_device__send_frames_to_client(pDevice, framesToSend, pData);

                    hr = mal_IAudioCaptureClient_ReleaseBuffer((mal_IAudioCaptureClient*)pDevice->wasapi.pCaptureClient, framesToSend);
                    if (FAILED(hr)) {
                        mal_post_error(pDevice, MAL_LOG_LEVEL_ERROR, "[WASAPI] WARNING: Failed to release internal buffer from capture device in preparation for sending new data to the client.", MAL_FAILED_TO_UNMAP_DEVICE_BUFFER);
                        break;
                    }

                    if (framesRemaining >= framesToSend) {
                        framesRemaining -= framesToSend;
                    } else {
                        framesRemaining = 0;
                    }
                }
            }
        }
    }

    return MAL_SUCCESS;
}

mal_result mal_context_uninit__wasapi(mal_context* pContext)
{
    mal_assert(pContext != NULL);
    mal_assert(pContext->backend == mal_backend_wasapi);
    (void)pContext;

    return MAL_SUCCESS;
}

mal_result mal_context_init__wasapi(mal_context* pContext)
{
    mal_assert(pContext != NULL);
    (void)pContext;

    mal_result result = MAL_SUCCESS;

#ifdef MAL_WIN32_DESKTOP
    // WASAPI is only supported in Vista SP1 and newer. The reason for SP1 and not the base version of Vista is that event-driven
    // exclusive mode does not work until SP1.
    mal_OSVERSIONINFOEXW osvi;
    mal_zero_object(&osvi);
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    osvi.dwMajorVersion = HIBYTE(_WIN32_WINNT_VISTA);
    osvi.dwMinorVersion = LOBYTE(_WIN32_WINNT_VISTA);
    osvi.wServicePackMajor = 1;
    if (VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, VerSetConditionMask(VerSetConditionMask(VerSetConditionMask(0, VER_MAJORVERSION, VER_GREATER_EQUAL), VER_MINORVERSION, VER_GREATER_EQUAL), VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL))) {
        result = MAL_SUCCESS;
    } else {
        result = MAL_NO_BACKEND;
    }
#endif

    if (result != MAL_SUCCESS) {
        return result;
    }

    pContext->onUninit              = mal_context_uninit__wasapi;
    pContext->onDeviceIDEqual       = mal_context_is_device_id_equal__wasapi;
    pContext->onEnumDevices         = mal_context_enumerate_devices__wasapi;
    pContext->onGetDeviceInfo       = mal_context_get_device_info__wasapi;
    pContext->onDeviceInit          = mal_device_init__wasapi;
    pContext->onDeviceUninit        = mal_device_uninit__wasapi;
    pContext->onDeviceReinit        = mal_device_reinit__wasapi;
    pContext->onDeviceStart         = mal_device__start_backend__wasapi;
    pContext->onDeviceStop          = mal_device__stop_backend__wasapi;
    pContext->onDeviceBreakMainLoop = mal_device__break_main_loop__wasapi;
    pContext->onDeviceMainLoop      = mal_device__main_loop__wasapi;

    return result;
}
#endif





mal_bool32 mal__is_channel_map_valid(const mal_channel* channelMap, mal_uint32 channels)
{
    // A blank channel map should be allowed, in which case it should use an appropriate default which will depend on context.
    if (channelMap[0] != MAL_CHANNEL_NONE) {
        if (channels == 0) {
            return MAL_FALSE;   // No channels.
        }

        // A channel cannot be present in the channel map more than once.
        for (mal_uint32 iChannel = 0; iChannel < channels; ++iChannel) {
            for (mal_uint32 jChannel = iChannel + 1; jChannel < channels; ++jChannel) {
                if (channelMap[iChannel] == channelMap[jChannel]) {
                    return MAL_FALSE;
                }
            }
        }
    }

    return MAL_TRUE;
}


void mal_device__post_init_setup(mal_device* pDevice)
{
    mal_assert(pDevice != NULL);

    // Make sure the internal channel map was set correctly by the backend. If it's not valid, just fall back to defaults.
    if (!mal_channel_map_valid(pDevice->internalChannels, pDevice->internalChannelMap)) {
        mal_get_standard_channel_map(mal_standard_channel_map_default, pDevice->internalChannels, pDevice->internalChannelMap);
    }


    // If the format/channels/rate is using defaults we need to set these to be the same as the internal config.
    if (pDevice->usingDefaultFormat) {
        pDevice->format = pDevice->internalFormat;
    }
    if (pDevice->usingDefaultChannels) {
        pDevice->channels = pDevice->internalChannels;
    }
    if (pDevice->usingDefaultSampleRate) {
        pDevice->sampleRate = pDevice->internalSampleRate;
    }
    if (pDevice->usingDefaultChannelMap) {
        mal_copy_memory(pDevice->channelMap, pDevice->internalChannelMap, sizeof(pDevice->channelMap));
    }

    // Buffer size. The backend will have set bufferSizeInFrames. We need to calculate bufferSizeInMilliseconds here.
    pDevice->bufferSizeInMilliseconds = pDevice->bufferSizeInFrames / (pDevice->internalSampleRate/1000);


    // We need a DSP object which is where samples are moved through in order to convert them to the
    // format required by the backend.
    mal_dsp_config dspConfig = mal_dsp_config_init_new();
    dspConfig.neverConsumeEndOfInput = MAL_TRUE;
    dspConfig.pUserData = pDevice;
    if (pDevice->type == mal_device_type_playback) {
        dspConfig.formatIn      = pDevice->format;
        dspConfig.channelsIn    = pDevice->channels;
        dspConfig.sampleRateIn  = pDevice->sampleRate;
        mal_copy_memory(dspConfig.channelMapIn, pDevice->channelMap, sizeof(dspConfig.channelMapIn));
        dspConfig.formatOut     = pDevice->internalFormat;
        dspConfig.channelsOut   = pDevice->internalChannels;
        dspConfig.sampleRateOut = pDevice->internalSampleRate;
        mal_copy_memory(dspConfig.channelMapOut, pDevice->internalChannelMap, sizeof(dspConfig.channelMapOut));
        dspConfig.onRead = mal_device__on_read_from_client;
        mal_dsp_init(&dspConfig, &pDevice->dsp);
    } else {
        dspConfig.formatIn      = pDevice->internalFormat;
        dspConfig.channelsIn    = pDevice->internalChannels;
        dspConfig.sampleRateIn  = pDevice->internalSampleRate;
        mal_copy_memory(dspConfig.channelMapIn, pDevice->internalChannelMap, sizeof(dspConfig.channelMapIn));
        dspConfig.formatOut     = pDevice->format;
        dspConfig.channelsOut   = pDevice->channels;
        dspConfig.sampleRateOut = pDevice->sampleRate;
        mal_copy_memory(dspConfig.channelMapOut, pDevice->channelMap, sizeof(dspConfig.channelMapOut));
        dspConfig.onRead = mal_device__on_read_from_device;
        mal_dsp_init(&dspConfig, &pDevice->dsp);
    }
}


mal_thread_result MAL_THREADCALL mal_worker_thread(void* pData)
{
    mal_device* pDevice = (mal_device*)pData;
    mal_assert(pDevice != NULL);

#ifdef MAL_WIN32
    mal_CoInitializeEx(pDevice->pContext, NULL, MAL_COINIT_VALUE);
#endif

    // When the device is being initialized it's initial state is set to MAL_STATE_UNINITIALIZED. Before returning from
    // mal_device_init(), the state needs to be set to something valid. In mini_al the device's default state immediately
    // after initialization is stopped, so therefore we need to mark the device as such. mini_al will wait on the worker
    // thread to signal an event to know when the worker thread is ready for action.
    mal_device__set_state(pDevice, MAL_STATE_STOPPED);
    mal_event_signal(&pDevice->stopEvent);

    for (;;) {
        // We wait on an event to know when something has requested that the device be started and the main loop entered.
        mal_event_wait(&pDevice->wakeupEvent);

        // Default result code.
        pDevice->workResult = MAL_SUCCESS;

        // If the reason for the wake up is that we are terminating, just break from the loop.
        if (mal_device__get_state(pDevice) == MAL_STATE_UNINITIALIZED) {
            break;
        }

        // Getting to this point means the device is wanting to get started. The function that has requested that the device
        // be started will be waiting on an event (pDevice->startEvent) which means we need to make sure we signal the event
        // in both the success and error case. It's important that the state of the device is set _before_ signaling the event.
        mal_assert(mal_device__get_state(pDevice) == MAL_STATE_STARTING);

        pDevice->workResult = pDevice->pContext->onDeviceStart(pDevice);
        if (pDevice->workResult != MAL_SUCCESS) {
            mal_device__set_state(pDevice, MAL_STATE_STOPPED);
            mal_event_signal(&pDevice->startEvent);
            continue;
        }

        // At this point the device should be started.
        mal_device__set_state(pDevice, MAL_STATE_STARTED);
        mal_event_signal(&pDevice->startEvent);


        // Now we just enter the main loop. When the main loop is terminated the device needs to be marked as stopped. This can
        // be broken with mal_device__break_main_loop().
        mal_result mainLoopResult = pDevice->pContext->onDeviceMainLoop(pDevice);
        if (mainLoopResult != MAL_SUCCESS && pDevice->isDefaultDevice && mal_device__get_state(pDevice) == MAL_STATE_STARTED && !pDevice->exclusiveMode) {
            // Something has failed during the main loop. It could be that the device has been lost. If it's the default device,
            // we can try switching over to the new default device by uninitializing and reinitializing.
            mal_result reinitResult = MAL_ERROR;
            if (pDevice->pContext->onDeviceReinit) {
                reinitResult = pDevice->pContext->onDeviceReinit(pDevice);
            } else {
                pDevice->pContext->onDeviceStop(pDevice);
                mal_device__set_state(pDevice, MAL_STATE_STOPPED);

                pDevice->pContext->onDeviceUninit(pDevice);
                mal_device__set_state(pDevice, MAL_STATE_UNINITIALIZED);

                reinitResult = pDevice->pContext->onDeviceInit(pDevice->pContext, pDevice->type, NULL, &pDevice->initConfig, pDevice);
            }

            // Perform the post initialization setup just in case the data conversion pipeline needs to be reinitialized.
            if (reinitResult == MAL_SUCCESS) {
                mal_device__post_init_setup(pDevice);
            }

            // If reinitialization was successful, loop back to the start.
            if (reinitResult == MAL_SUCCESS) {
                mal_device__set_state(pDevice, MAL_STATE_STARTING); // <-- The device is restarting.
                mal_event_signal(&pDevice->wakeupEvent);
                continue;
            }
        }


        // Getting here means we have broken from the main loop which happens the application has requested that device be stopped. Note that this
        // may have actually already happened above if the device was lost and mini_al has attempted to re-initialize the device. In this case we
        // don't want to be doing this a second time.
        if (mal_device__get_state(pDevice) != MAL_STATE_UNINITIALIZED) {
            pDevice->pContext->onDeviceStop(pDevice);
        }

        // After the device has stopped, make sure an event is posted.
        mal_stop_proc onStop = pDevice->onStop;
        if (onStop) {
            onStop(pDevice);
        }

        // A function somewhere is waiting for the device to have stopped for real so we need to signal an event to allow it to continue. Note that
        // it's possible that the device has been uninitialized which means we need to _not_ change the status to stopped. We cannot go from an
        // uninitialized state to stopped state.
        if (mal_device__get_state(pDevice) != MAL_STATE_UNINITIALIZED) {
            mal_device__set_state(pDevice, MAL_STATE_STOPPED);
            mal_event_signal(&pDevice->stopEvent);
        }
    }

    // Make sure we aren't continuously waiting on a stop event.
    mal_event_signal(&pDevice->stopEvent);  // <-- Is this still needed?

#ifdef MAL_WIN32
    mal_CoUninitialize(pDevice->pContext);
#endif

    return (mal_thread_result)0;
}


// Helper for determining whether or not the given device is initialized.
mal_bool32 mal_device__is_initialized(mal_device* pDevice)
{
    if (pDevice == NULL) return MAL_FALSE;
    return mal_device__get_state(pDevice) != MAL_STATE_UNINITIALIZED;
}


#ifdef MAL_WIN32
mal_result mal_context_uninit_backend_apis__win32(mal_context* pContext)
{
    mal_CoUninitialize(pContext);
    mal_dlclose(pContext->win32.hUser32DLL);
    mal_dlclose(pContext->win32.hOle32DLL);
    mal_dlclose(pContext->win32.hAdvapi32DLL);

    return MAL_SUCCESS;
}

mal_result mal_context_init_backend_apis__win32(mal_context* pContext)
{
#ifdef MAL_WIN32_DESKTOP
    // Ole32.dll
    pContext->win32.hOle32DLL = mal_dlopen("ole32.dll");
    if (pContext->win32.hOle32DLL == NULL) {
        return MAL_FAILED_TO_INIT_BACKEND;
    }

    pContext->win32.CoInitializeEx   = (mal_proc)mal_dlsym(pContext->win32.hOle32DLL, "CoInitializeEx");
    pContext->win32.CoUninitialize   = (mal_proc)mal_dlsym(pContext->win32.hOle32DLL, "CoUninitialize");
    pContext->win32.CoCreateInstance = (mal_proc)mal_dlsym(pContext->win32.hOle32DLL, "CoCreateInstance");
    pContext->win32.CoTaskMemFree    = (mal_proc)mal_dlsym(pContext->win32.hOle32DLL, "CoTaskMemFree");
    pContext->win32.PropVariantClear = (mal_proc)mal_dlsym(pContext->win32.hOle32DLL, "PropVariantClear");
    pContext->win32.StringFromGUID2  = (mal_proc)mal_dlsym(pContext->win32.hOle32DLL, "StringFromGUID2");


    // User32.dll
    pContext->win32.hUser32DLL = mal_dlopen("user32.dll");
    if (pContext->win32.hUser32DLL == NULL) {
        return MAL_FAILED_TO_INIT_BACKEND;
    }

    pContext->win32.GetForegroundWindow = (mal_proc)mal_dlsym(pContext->win32.hUser32DLL, "GetForegroundWindow");
    pContext->win32.GetDesktopWindow    = (mal_proc)mal_dlsym(pContext->win32.hUser32DLL, "GetDesktopWindow");


    // Advapi32.dll
    pContext->win32.hAdvapi32DLL = mal_dlopen("advapi32.dll");
    if (pContext->win32.hAdvapi32DLL == NULL) {
        return MAL_FAILED_TO_INIT_BACKEND;
    }

    pContext->win32.RegOpenKeyExA    = (mal_proc)mal_dlsym(pContext->win32.hAdvapi32DLL, "RegOpenKeyExA");
    pContext->win32.RegCloseKey      = (mal_proc)mal_dlsym(pContext->win32.hAdvapi32DLL, "RegCloseKey");
    pContext->win32.RegQueryValueExA = (mal_proc)mal_dlsym(pContext->win32.hAdvapi32DLL, "RegQueryValueExA");
#endif

    mal_CoInitializeEx(pContext, NULL, MAL_COINIT_VALUE);
    return MAL_SUCCESS;
}
#else
mal_result mal_context_uninit_backend_apis__nix(mal_context* pContext)
{
#if defined(MAL_USE_RUNTIME_LINKING_FOR_PTHREAD) && !defined(MAL_NO_RUNTIME_LINKING)
    mal_dlclose(pContext->posix.pthreadSO);
#else
    (void)pContext;
#endif

    return MAL_SUCCESS;
}

mal_result mal_context_init_backend_apis__nix(mal_context* pContext)
{
    // pthread
#if defined(MAL_USE_RUNTIME_LINKING_FOR_PTHREAD) && !defined(MAL_NO_RUNTIME_LINKING)
    const char* libpthreadFileNames[] = {
        "libpthread.so",
        "libpthread.so.0",
        "libpthread.dylib"
    };

    for (size_t i = 0; i < sizeof(libpthreadFileNames) / sizeof(libpthreadFileNames[0]); ++i) {
        pContext->posix.pthreadSO = mal_dlopen(libpthreadFileNames[i]);
        if (pContext->posix.pthreadSO != NULL) {
            break;
        }
    }

    if (pContext->posix.pthreadSO == NULL) {
        return MAL_FAILED_TO_INIT_BACKEND;
    }

    pContext->posix.pthread_create              = (mal_proc)mal_dlsym(pContext->posix.pthreadSO, "pthread_create");
    pContext->posix.pthread_join                = (mal_proc)mal_dlsym(pContext->posix.pthreadSO, "pthread_join");
    pContext->posix.pthread_mutex_init          = (mal_proc)mal_dlsym(pContext->posix.pthreadSO, "pthread_mutex_init");
    pContext->posix.pthread_mutex_destroy       = (mal_proc)mal_dlsym(pContext->posix.pthreadSO, "pthread_mutex_destroy");
    pContext->posix.pthread_mutex_lock          = (mal_proc)mal_dlsym(pContext->posix.pthreadSO, "pthread_mutex_lock");
    pContext->posix.pthread_mutex_unlock        = (mal_proc)mal_dlsym(pContext->posix.pthreadSO, "pthread_mutex_unlock");
    pContext->posix.pthread_cond_init           = (mal_proc)mal_dlsym(pContext->posix.pthreadSO, "pthread_cond_init");
    pContext->posix.pthread_cond_destroy        = (mal_proc)mal_dlsym(pContext->posix.pthreadSO, "pthread_cond_destroy");
    pContext->posix.pthread_cond_wait           = (mal_proc)mal_dlsym(pContext->posix.pthreadSO, "pthread_cond_wait");
    pContext->posix.pthread_cond_signal         = (mal_proc)mal_dlsym(pContext->posix.pthreadSO, "pthread_cond_signal");
    pContext->posix.pthread_attr_init           = (mal_proc)mal_dlsym(pContext->posix.pthreadSO, "pthread_attr_init");
    pContext->posix.pthread_attr_destroy        = (mal_proc)mal_dlsym(pContext->posix.pthreadSO, "pthread_attr_destroy");
    pContext->posix.pthread_attr_setschedpolicy = (mal_proc)mal_dlsym(pContext->posix.pthreadSO, "pthread_attr_setschedpolicy");
    pContext->posix.pthread_attr_getschedparam  = (mal_proc)mal_dlsym(pContext->posix.pthreadSO, "pthread_attr_getschedparam");
    pContext->posix.pthread_attr_setschedparam  = (mal_proc)mal_dlsym(pContext->posix.pthreadSO, "pthread_attr_setschedparam");
#else
    pContext->posix.pthread_create              = (mal_proc)pthread_create;
    pContext->posix.pthread_join                = (mal_proc)pthread_join;
    pContext->posix.pthread_mutex_init          = (mal_proc)pthread_mutex_init;
    pContext->posix.pthread_mutex_destroy       = (mal_proc)pthread_mutex_destroy;
    pContext->posix.pthread_mutex_lock          = (mal_proc)pthread_mutex_lock;
    pContext->posix.pthread_mutex_unlock        = (mal_proc)pthread_mutex_unlock;
    pContext->posix.pthread_cond_init           = (mal_proc)pthread_cond_init;
    pContext->posix.pthread_cond_destroy        = (mal_proc)pthread_cond_destroy;
    pContext->posix.pthread_cond_wait           = (mal_proc)pthread_cond_wait;
    pContext->posix.pthread_cond_signal         = (mal_proc)pthread_cond_signal;
    pContext->posix.pthread_attr_init           = (mal_proc)pthread_attr_init;
    pContext->posix.pthread_attr_destroy        = (mal_proc)pthread_attr_destroy;
#if !defined(__EMSCRIPTEN__)
    pContext->posix.pthread_attr_setschedpolicy = (mal_proc)pthread_attr_setschedpolicy;
    pContext->posix.pthread_attr_getschedparam  = (mal_proc)pthread_attr_getschedparam;
    pContext->posix.pthread_attr_setschedparam  = (mal_proc)pthread_attr_setschedparam;
#endif
#endif

    return MAL_SUCCESS;
}
#endif

mal_result mal_context_init_backend_apis(mal_context* pContext)
{
    mal_result result;
#ifdef MAL_WIN32
    result = mal_context_init_backend_apis__win32(pContext);
#else
    result = mal_context_init_backend_apis__nix(pContext);
#endif

    return result;
}

mal_result mal_context_uninit_backend_apis(mal_context* pContext)
{
    mal_result result;
#ifdef MAL_WIN32
    result = mal_context_uninit_backend_apis__win32(pContext);
#else
    result = mal_context_uninit_backend_apis__nix(pContext);
#endif

    return result;
}


mal_bool32 mal_context_is_backend_asynchronous(mal_context* pContext)
{
    return pContext->isBackendAsynchronous;
}

mal_result mal_context_init(const mal_backend backends[], mal_uint32 backendCount, const mal_context_config* pConfig, mal_context* pContext)
{
    if (pContext == NULL) {
        return MAL_INVALID_ARGS;
    }

    mal_zero_object(pContext);

    // Always make sure the config is set first to ensure properties are available as soon as possible.
    if (pConfig != NULL) {
        pContext->config = *pConfig;
    } else {
        pContext->config = mal_context_config_init(NULL);
    }

    // Backend APIs need to be initialized first. This is where external libraries will be loaded and linked.
    mal_result result = mal_context_init_backend_apis(pContext);
    if (result != MAL_SUCCESS) {
        return result;
    }

    mal_backend* pBackendsToIterate = (mal_backend*)backends;
    mal_uint32 backendsToIterateCount = backendCount;
    if (pBackendsToIterate == NULL) {
        pBackendsToIterate = (mal_backend*)g_malDefaultBackends;
        backendsToIterateCount = mal_countof(g_malDefaultBackends);
    }

    mal_assert(pBackendsToIterate != NULL);

    for (mal_uint32 iBackend = 0; iBackend < backendsToIterateCount; ++iBackend) {
        mal_backend backend = pBackendsToIterate[iBackend];

        result = MAL_NO_BACKEND;
        switch (backend) {
            case mal_backend_wasapi:
            {
                result = mal_context_init__wasapi(pContext);
            } break;

            default: break;
        }

        // If this iteration was successful, return.
        if (result == MAL_SUCCESS) {
            result = mal_mutex_init(pContext, &pContext->deviceEnumLock);
            if (result != MAL_SUCCESS) {
                mal_context_post_error(pContext, NULL, MAL_LOG_LEVEL_WARNING, "Failed to initialize mutex for device enumeration. mal_context_get_devices() is not thread safe.", MAL_FAILED_TO_CREATE_MUTEX);
            }
            result = mal_mutex_init(pContext, &pContext->deviceInfoLock);
            if (result != MAL_SUCCESS) {
                mal_context_post_error(pContext, NULL, MAL_LOG_LEVEL_WARNING, "Failed to initialize mutex for device info retrieval. mal_context_get_device_info() is not thread safe.", MAL_FAILED_TO_CREATE_MUTEX);
            }

#ifdef MAL_DEBUG_OUTPUT
            printf("[mini_al] Endian:  %s\n", mal_is_little_endian() ? "LE" : "BE");
            printf("[mini_al] SSE2:    %s\n", mal_has_sse2()    ? "YES" : "NO");
            printf("[mini_al] AVX2:    %s\n", mal_has_avx2()    ? "YES" : "NO");
            printf("[mini_al] AVX512F: %s\n", mal_has_avx512f() ? "YES" : "NO");
            printf("[mini_al] NEON:    %s\n", mal_has_neon()    ? "YES" : "NO");
#endif

            pContext->backend = backend;
            return result;
        }
    }

    // If we get here it means an error occurred.
    mal_zero_object(pContext);  // Safety.
    return MAL_NO_BACKEND;
}

mal_result mal_context_uninit(mal_context* pContext)
{
    if (pContext == NULL) {
        return MAL_INVALID_ARGS;
    }

    pContext->onUninit(pContext);

    mal_context_uninit_backend_apis(pContext);
    mal_mutex_uninit(&pContext->deviceEnumLock);
    mal_mutex_uninit(&pContext->deviceInfoLock);
    mal_free(pContext->pDeviceInfos);

    return MAL_SUCCESS;
}


mal_result mal_context_enumerate_devices(mal_context* pContext, mal_enum_devices_callback_proc callback, void* pUserData)
{
    if (pContext == NULL || pContext->onEnumDevices == NULL || callback == NULL) {
        return MAL_INVALID_ARGS;
    }

    mal_result result;
    mal_mutex_lock(&pContext->deviceEnumLock);
    {
        result = pContext->onEnumDevices(pContext, callback, pUserData);
    }
    mal_mutex_unlock(&pContext->deviceEnumLock);

    return result;
}


mal_bool32 mal_context_get_devices__enum_callback(mal_context* pContext, mal_device_type type, const mal_device_info* pInfo, void* pUserData)
{
    (void)pUserData;

    // We need to insert the device info into our main internal buffer. Where it goes depends on the device type. If it's a capture device
    // it's just appended to the end. If it's a playback device it's inserted just before the first capture device.

    // First make sure we have room. Since the number of devices we add to the list is usually relatively small I've decided to use a
    // simple fixed size increment for buffer expansion.
    const mal_uint32 bufferExpansionCount = 2;
    const mal_uint32 totalDeviceInfoCount = pContext->playbackDeviceInfoCount + pContext->captureDeviceInfoCount;

    if (pContext->deviceInfoCapacity >= totalDeviceInfoCount) {
        mal_uint32 newCapacity = totalDeviceInfoCount + bufferExpansionCount;
        mal_device_info* pNewInfos = (mal_device_info*)mal_realloc(pContext->pDeviceInfos, sizeof(*pContext->pDeviceInfos)*newCapacity);
        if (pNewInfos == NULL) {
            return MAL_FALSE;   // Out of memory.
        }

        pContext->pDeviceInfos = pNewInfos;
        pContext->deviceInfoCapacity = newCapacity;
    }

    if (type == mal_device_type_playback) {
        // Playback. Insert just before the first capture device.

        // The first thing to do is move all of the capture devices down a slot.
        mal_uint32 iFirstCaptureDevice = pContext->playbackDeviceInfoCount;
        for (size_t iCaptureDevice = totalDeviceInfoCount; iCaptureDevice > iFirstCaptureDevice; --iCaptureDevice) {
            pContext->pDeviceInfos[iCaptureDevice] = pContext->pDeviceInfos[iCaptureDevice-1];
        }

        // Now just insert where the first capture device was before moving it down a slot.
        pContext->pDeviceInfos[iFirstCaptureDevice] = *pInfo;
        pContext->playbackDeviceInfoCount += 1;
    } else {
        // Capture. Insert at the end.
        pContext->pDeviceInfos[totalDeviceInfoCount] = *pInfo;
        pContext->captureDeviceInfoCount += 1;
    }

    return MAL_TRUE;
}

mal_result mal_context_get_devices(mal_context* pContext, mal_device_info** ppPlaybackDeviceInfos, mal_uint32* pPlaybackDeviceCount, mal_device_info** ppCaptureDeviceInfos, mal_uint32* pCaptureDeviceCount)
{
    // Safety.
    if (ppPlaybackDeviceInfos != NULL) *ppPlaybackDeviceInfos = NULL;
    if (pPlaybackDeviceCount  != NULL) *pPlaybackDeviceCount  = 0;
    if (ppCaptureDeviceInfos  != NULL) *ppCaptureDeviceInfos  = NULL;
    if (pCaptureDeviceCount   != NULL) *pCaptureDeviceCount   = 0;

    if (pContext == NULL || pContext->onEnumDevices == NULL) {
        return MAL_INVALID_ARGS;
    }

    // Note that we don't use mal_context_enumerate_devices() here because we want to do locking at a higher level.
    mal_result result;
    mal_mutex_lock(&pContext->deviceEnumLock);
    {
        // Reset everything first.
        pContext->playbackDeviceInfoCount = 0;
        pContext->captureDeviceInfoCount = 0;

        // Now enumerate over available devices.
        result = pContext->onEnumDevices(pContext, mal_context_get_devices__enum_callback, NULL);
        if (result == MAL_SUCCESS) {
            // Playback devices.
            if (ppPlaybackDeviceInfos != NULL) {
                *ppPlaybackDeviceInfos = pContext->pDeviceInfos;
            }
            if (pPlaybackDeviceCount != NULL) {
                *pPlaybackDeviceCount = pContext->playbackDeviceInfoCount;
            }

            // Capture devices.
            if (ppCaptureDeviceInfos != NULL) {
                *ppCaptureDeviceInfos = pContext->pDeviceInfos + pContext->playbackDeviceInfoCount; // Capture devices come after playback devices.
            }
            if (pCaptureDeviceCount != NULL) {
                *pCaptureDeviceCount = pContext->captureDeviceInfoCount;
            }
        }
    }
    mal_mutex_unlock(&pContext->deviceEnumLock);

    return result;
}

mal_result mal_context_get_device_info(mal_context* pContext, mal_device_type type, const mal_device_id* pDeviceID, mal_share_mode shareMode, mal_device_info* pDeviceInfo)
{
    // NOTE: Do not clear pDeviceInfo on entry. The reason is the pDeviceID may actually point to pDeviceInfo->id which will break things.
    if (pContext == NULL || pDeviceInfo == NULL) {
        return MAL_INVALID_ARGS;
    }

    mal_device_info deviceInfo;
    mal_zero_object(&deviceInfo);

    // Help the backend out by copying over the device ID if we have one.
    if (pDeviceID != NULL) {
        mal_copy_memory(&deviceInfo.id, pDeviceID, sizeof(*pDeviceID));
    }

    // The backend may have an optimized device info retrieval function. If so, try that first.
    if (pContext->onGetDeviceInfo != NULL) {
        mal_result result;
        mal_mutex_lock(&pContext->deviceInfoLock);
        {
            result = pContext->onGetDeviceInfo(pContext, type, pDeviceID, shareMode, &deviceInfo);
        }
        mal_mutex_unlock(&pContext->deviceInfoLock);

        // Clamp ranges.
        deviceInfo.minChannels   = mal_max(deviceInfo.minChannels,   MAL_MIN_CHANNELS);
        deviceInfo.maxChannels   = mal_min(deviceInfo.maxChannels,   MAL_MAX_CHANNELS);
        deviceInfo.minSampleRate = mal_max(deviceInfo.minSampleRate, MAL_MIN_SAMPLE_RATE);
        deviceInfo.maxSampleRate = mal_min(deviceInfo.maxSampleRate, MAL_MAX_SAMPLE_RATE);

        *pDeviceInfo = deviceInfo;
        return result;
    }

    // Getting here means onGetDeviceInfo has not been set.
    return MAL_ERROR;
}


mal_result mal_device_init(mal_context* pContext, mal_device_type type, mal_device_id* pDeviceID, const mal_device_config* pConfig, void* pUserData, mal_device* pDevice)
{
    if (pContext == NULL) {
        return mal_device_init_ex(NULL, 0, NULL, type, pDeviceID, pConfig, pUserData, pDevice);
    }


    if (pDevice == NULL) {
        return mal_post_error(pDevice, MAL_LOG_LEVEL_ERROR, "mal_device_init() called with invalid arguments (pDevice == NULL).",  MAL_INVALID_ARGS);
    }

    // The config is allowed to be NULL, in which case we default to mal_device_config_init_default().
    mal_device_config config;
    if (pConfig == NULL) {
        config = mal_device_config_init_default();
    } else {
        config = *pConfig;
    }

    // Basic config validation.
    if (config.channels > MAL_MAX_CHANNELS) {
        return mal_post_error(pDevice, MAL_LOG_LEVEL_ERROR, "mal_device_init() called with an invalid config. Channel count cannot exceed 32.", MAL_INVALID_DEVICE_CONFIG);
    }
    if (!mal__is_channel_map_valid(config.channelMap, config.channels)) {
        return mal_post_error(pDevice, MAL_LOG_LEVEL_ERROR, "mal_device_init() called with invalid config. Channel map is invalid.", MAL_INVALID_DEVICE_CONFIG);
    }


    mal_zero_object(pDevice);
    pDevice->pContext = pContext;
    pDevice->initConfig = config;

    // Set the user data and log callback ASAP to ensure it is available for the entire initialization process.
    pDevice->pUserData = pUserData;
    pDevice->onStop = config.onStopCallback;
    pDevice->onSend = config.onSendCallback;
    pDevice->onRecv = config.onRecvCallback;

    if (((size_t)pDevice % sizeof(pDevice)) != 0) {
        if (pContext->config.onLog) {
            pContext->config.onLog(pContext, pDevice, "WARNING: mal_device_init() called for a device that is not properly aligned. Thread safety is not supported.");
        }
    }

    if (pDeviceID == NULL) {
        pDevice->isDefaultDevice = MAL_TRUE;
    }


    // When passing in 0 for the format/channels/rate/chmap it means the device will be using whatever is chosen by the backend. If everything is set
    // to defaults it means the format conversion pipeline will run on a fast path where data transfer is just passed straight through to the backend.
    if (config.format == mal_format_unknown) {
        config.format = MAL_DEFAULT_FORMAT;
        pDevice->usingDefaultFormat = MAL_TRUE;
    }
    if (config.channels == 0) {
        config.channels = MAL_DEFAULT_CHANNELS;
        pDevice->usingDefaultChannels = MAL_TRUE;
    }
    if (config.sampleRate == 0) {
        config.sampleRate = MAL_DEFAULT_SAMPLE_RATE;
        pDevice->usingDefaultSampleRate = MAL_TRUE;
    }
    if (config.channelMap[0] == MAL_CHANNEL_NONE) {
        pDevice->usingDefaultChannelMap = MAL_TRUE;
    }


    // Default buffer size.
    if (config.bufferSizeInMilliseconds == 0 && config.bufferSizeInFrames == 0) {
        config.bufferSizeInMilliseconds = (config.performanceProfile == mal_performance_profile_low_latency) ? MAL_BASE_BUFFER_SIZE_IN_MILLISECONDS_LOW_LATENCY : MAL_BASE_BUFFER_SIZE_IN_MILLISECONDS_CONSERVATIVE;
        pDevice->usingDefaultBufferSize = MAL_TRUE;
    }

    // Default periods.
    if (config.periods == 0) {
        config.periods = MAL_DEFAULT_PERIODS;
        pDevice->usingDefaultPeriods = MAL_TRUE;
    }

    pDevice->type = type;
    pDevice->format = config.format;
    pDevice->channels = config.channels;
    pDevice->sampleRate = config.sampleRate;
    mal_copy_memory(pDevice->channelMap, config.channelMap, sizeof(config.channelMap[0]) * config.channels);
    pDevice->bufferSizeInFrames = config.bufferSizeInFrames;
    pDevice->bufferSizeInMilliseconds = config.bufferSizeInMilliseconds;
    pDevice->periods = config.periods;

    // The internal format, channel count and sample rate can be modified by the backend.
    pDevice->internalFormat = pDevice->format;
    pDevice->internalChannels = pDevice->channels;
    pDevice->internalSampleRate = pDevice->sampleRate;
    mal_copy_memory(pDevice->internalChannelMap, pDevice->channelMap, sizeof(pDevice->channelMap));

    if (mal_mutex_init(pContext, &pDevice->lock) != MAL_SUCCESS) {
        return mal_post_error(pDevice, MAL_LOG_LEVEL_ERROR, "Failed to create mutex.", MAL_FAILED_TO_CREATE_MUTEX);
    }

    // When the device is started, the worker thread is the one that does the actual startup of the backend device. We
    // use a semaphore to wait for the background thread to finish the work. The same applies for stopping the device.
    //
    // Each of these semaphores is released internally by the worker thread when the work is completed. The start
    // semaphore is also used to wake up the worker thread.
    if (mal_event_init(pContext, &pDevice->wakeupEvent) != MAL_SUCCESS) {
        mal_mutex_uninit(&pDevice->lock);
        return mal_post_error(pDevice, MAL_LOG_LEVEL_ERROR, "Failed to create worker thread wakeup event.", MAL_FAILED_TO_CREATE_EVENT);
    }
    if (mal_event_init(pContext, &pDevice->startEvent) != MAL_SUCCESS) {
        mal_event_uninit(&pDevice->wakeupEvent);
        mal_mutex_uninit(&pDevice->lock);
        return mal_post_error(pDevice, MAL_LOG_LEVEL_ERROR, "Failed to create worker thread start event.", MAL_FAILED_TO_CREATE_EVENT);
    }
    if (mal_event_init(pContext, &pDevice->stopEvent) != MAL_SUCCESS) {
        mal_event_uninit(&pDevice->startEvent);
        mal_event_uninit(&pDevice->wakeupEvent);
        mal_mutex_uninit(&pDevice->lock);
        return mal_post_error(pDevice, MAL_LOG_LEVEL_ERROR, "Failed to create worker thread stop event.", MAL_FAILED_TO_CREATE_EVENT);
    }


    mal_result result = pContext->onDeviceInit(pContext, type, pDeviceID, &config, pDevice);
    if (result != MAL_SUCCESS) {
        return MAL_NO_BACKEND;  // The error message will have been posted with mal_post_error() by the source of the error so don't bother calling it here.
    }

    mal_device__post_init_setup(pDevice);


    // If the backend did not fill out a name for the device, try a generic method.
    if (pDevice->name[0] == '\0') {
        if (mal_context__try_get_device_name_by_id(pContext, type, pDeviceID, pDevice->name, sizeof(pDevice->name)) != MAL_SUCCESS) {
            // We failed to get the device name, so fall back to some generic names.
            if (pDeviceID == NULL) {
                if (type == mal_device_type_playback) {
                    mal_strncpy_s(pDevice->name, sizeof(pDevice->name), MAL_DEFAULT_PLAYBACK_DEVICE_NAME, (size_t)-1);
                } else {
                    mal_strncpy_s(pDevice->name, sizeof(pDevice->name), MAL_DEFAULT_CAPTURE_DEVICE_NAME, (size_t)-1);
                }
            } else {
                if (type == mal_device_type_playback) {
                    mal_strncpy_s(pDevice->name, sizeof(pDevice->name), "Playback Device", (size_t)-1);
                } else {
                    mal_strncpy_s(pDevice->name, sizeof(pDevice->name), "Capture Device", (size_t)-1);
                }
            }
        }
    }


    // Some backends don't require the worker thread.
    if (!mal_context_is_backend_asynchronous(pContext)) {
        // The worker thread.
        if (mal_thread_create(pContext, &pDevice->thread, mal_worker_thread, pDevice) != MAL_SUCCESS) {
            mal_device_uninit(pDevice);
            return mal_post_error(pDevice, MAL_LOG_LEVEL_ERROR, "Failed to create worker thread.", MAL_FAILED_TO_CREATE_THREAD);
        }

        // Wait for the worker thread to put the device into it's stopped state for real.
        mal_event_wait(&pDevice->stopEvent);
    } else {
        mal_device__set_state(pDevice, MAL_STATE_STOPPED);
    }


#ifdef MAL_DEBUG_OUTPUT
    printf("[%s] %s (%s)\n", mal_get_backend_name(pDevice->pContext->backend), pDevice->name, (pDevice->type == mal_device_type_playback) ? "Playback" : "Capture");
    printf("  Format:      %s -> %s\n", mal_get_format_name(pDevice->format), mal_get_format_name(pDevice->internalFormat));
    printf("  Channels:    %d -> %d\n", pDevice->channels, pDevice->internalChannels);
    printf("  Sample Rate: %d -> %d\n", pDevice->sampleRate, pDevice->internalSampleRate);
    printf("  Conversion:\n");
    printf("    Pre Format Conversion:    %s\n", pDevice->dsp.isPreFormatConversionRequired  ? "YES" : "NO");
    printf("    Post Format Conversion:   %s\n", pDevice->dsp.isPostFormatConversionRequired ? "YES" : "NO");
    printf("    Channel Routing:          %s\n", pDevice->dsp.isChannelRoutingRequired       ? "YES" : "NO");
    printf("    SRC:                      %s\n", pDevice->dsp.isSRCRequired                  ? "YES" : "NO");
    printf("    Channel Routing at Start: %s\n", pDevice->dsp.isChannelRoutingAtStart        ? "YES" : "NO");
    printf("    Passthrough:              %s\n", pDevice->dsp.isPassthrough                  ? "YES" : "NO");
#endif


    mal_assert(mal_device__get_state(pDevice) == MAL_STATE_STOPPED);
    return MAL_SUCCESS;
}

mal_result mal_device_init_ex(const mal_backend backends[], mal_uint32 backendCount, const mal_context_config* pContextConfig, mal_device_type type, mal_device_id* pDeviceID, const mal_device_config* pConfig, void* pUserData, mal_device* pDevice)
{
    mal_context* pContext = (mal_context*)mal_malloc(sizeof(*pContext));
    if (pContext == NULL) {
        return MAL_OUT_OF_MEMORY;
    }

    mal_backend* pBackendsToIterate = (mal_backend*)backends;
    mal_uint32 backendsToIterateCount = backendCount;
    if (pBackendsToIterate == NULL) {
        pBackendsToIterate = (mal_backend*)g_malDefaultBackends;
        backendsToIterateCount = mal_countof(g_malDefaultBackends);
    }

    mal_result result = MAL_NO_BACKEND;

    for (mal_uint32 iBackend = 0; iBackend < backendsToIterateCount; ++iBackend) {
        result = mal_context_init(&pBackendsToIterate[iBackend], 1, pContextConfig, pContext);
        if (result == MAL_SUCCESS) {
            result = mal_device_init(pContext, type, pDeviceID, pConfig, pUserData, pDevice);
            if (result == MAL_SUCCESS) {
                break;  // Success.
            } else {
                mal_context_uninit(pContext);   // Failure.
            }
        }
    }

    if (result != MAL_SUCCESS) {
        mal_free(pContext);
        return result;
    }

    pDevice->isOwnerOfContext = MAL_TRUE;
    return result;
}

void mal_device_uninit(mal_device* pDevice)
{
    if (!mal_device__is_initialized(pDevice)) return;

    // Make sure the device is stopped first. The backends will probably handle this naturally,
    // but I like to do it explicitly for my own sanity.
    if (mal_device_is_started(pDevice)) {
        while (mal_device_stop(pDevice) == MAL_DEVICE_BUSY) {
            mal_sleep(1);
        }
    }

    // Putting the device into an uninitialized state will make the worker thread return.
    mal_device__set_state(pDevice, MAL_STATE_UNINITIALIZED);

    // Wake up the worker thread and wait for it to properly terminate.
    if (!mal_context_is_backend_asynchronous(pDevice->pContext)) {
        mal_event_signal(&pDevice->wakeupEvent);
        mal_thread_wait(&pDevice->thread);
    }

    pDevice->pContext->onDeviceUninit(pDevice);

    mal_event_uninit(&pDevice->stopEvent);
    mal_event_uninit(&pDevice->startEvent);
    mal_event_uninit(&pDevice->wakeupEvent);
    mal_mutex_uninit(&pDevice->lock);

    if (pDevice->isOwnerOfContext) {
        mal_context_uninit(pDevice->pContext);
        mal_free(pDevice->pContext);
    }

    mal_zero_object(pDevice);
}

void mal_device_set_recv_callback(mal_device* pDevice, mal_recv_proc proc)
{
    if (pDevice == NULL) return;
    mal_atomic_exchange_ptr(&pDevice->onRecv, proc);
}

void mal_device_set_send_callback(mal_device* pDevice, mal_send_proc proc)
{
    if (pDevice == NULL) return;
    mal_atomic_exchange_ptr(&pDevice->onSend, proc);
}

void mal_device_set_stop_callback(mal_device* pDevice, mal_stop_proc proc)
{
    if (pDevice == NULL) return;
    mal_atomic_exchange_ptr(&pDevice->onStop, proc);
}

mal_result mal_device_start(mal_device* pDevice)
{
    if (pDevice == NULL) return mal_post_error(pDevice, MAL_LOG_LEVEL_ERROR, "mal_device_start() called with invalid arguments (pDevice == NULL).", MAL_INVALID_ARGS);
    if (mal_device__get_state(pDevice) == MAL_STATE_UNINITIALIZED) return mal_post_error(pDevice, MAL_LOG_LEVEL_ERROR, "mal_device_start() called for an uninitialized device.", MAL_DEVICE_NOT_INITIALIZED);

    mal_result result = MAL_ERROR;
    mal_mutex_lock(&pDevice->lock);
    {
        // Be a bit more descriptive if the device is already started or is already in the process of starting. This is likely
        // a bug with the application.
        if (mal_device__get_state(pDevice) == MAL_STATE_STARTING) {
            mal_mutex_unlock(&pDevice->lock);
            return mal_post_error(pDevice, MAL_LOG_LEVEL_ERROR, "mal_device_start() called while another thread is already starting it.", MAL_DEVICE_ALREADY_STARTING);
        }
        if (mal_device__get_state(pDevice) == MAL_STATE_STARTED) {
            mal_mutex_unlock(&pDevice->lock);
            return mal_post_error(pDevice, MAL_LOG_LEVEL_ERROR, "mal_device_start() called for a device that's already started.", MAL_DEVICE_ALREADY_STARTED);
        }

        // The device needs to be in a stopped state. If it's not, we just let the caller know the device is busy.
        if (mal_device__get_state(pDevice) != MAL_STATE_STOPPED) {
            mal_mutex_unlock(&pDevice->lock);
            return mal_post_error(pDevice, MAL_LOG_LEVEL_ERROR, "mal_device_start() called while another thread is in the process of stopping it.", MAL_DEVICE_BUSY);
        }

        mal_device__set_state(pDevice, MAL_STATE_STARTING);

        // Asynchronous backends need to be handled differently.
        if (mal_context_is_backend_asynchronous(pDevice->pContext)) {
            result = pDevice->pContext->onDeviceStart(pDevice);
            if (result == MAL_SUCCESS) {
                mal_device__set_state(pDevice, MAL_STATE_STARTED);
            }
        } else {
            // Synchronous backends are started by signaling an event that's being waited on in the worker thread. We first wake up the
            // thread and then wait for the start event.
            mal_event_signal(&pDevice->wakeupEvent);

            // Wait for the worker thread to finish starting the device. Note that the worker thread will be the one
            // who puts the device into the started state. Don't call mal_device__set_state() here.
            mal_event_wait(&pDevice->startEvent);
            result = pDevice->workResult;
        }
    }
    mal_mutex_unlock(&pDevice->lock);

    return result;
}

mal_result mal_device_stop(mal_device* pDevice)
{
    if (pDevice == NULL) return mal_post_error(pDevice, MAL_LOG_LEVEL_ERROR, "mal_device_stop() called with invalid arguments (pDevice == NULL).", MAL_INVALID_ARGS);
    if (mal_device__get_state(pDevice) == MAL_STATE_UNINITIALIZED) return mal_post_error(pDevice, MAL_LOG_LEVEL_ERROR, "mal_device_stop() called for an uninitialized device.", MAL_DEVICE_NOT_INITIALIZED);

    mal_result result = MAL_ERROR;
    mal_mutex_lock(&pDevice->lock);
    {
        // Be a bit more descriptive if the device is already stopped or is already in the process of stopping. This is likely
        // a bug with the application.
        if (mal_device__get_state(pDevice) == MAL_STATE_STOPPING) {
            mal_mutex_unlock(&pDevice->lock);
            return mal_post_error(pDevice, MAL_LOG_LEVEL_ERROR, "mal_device_stop() called while another thread is already stopping it.", MAL_DEVICE_ALREADY_STOPPING);
        }
        if (mal_device__get_state(pDevice) == MAL_STATE_STOPPED) {
            mal_mutex_unlock(&pDevice->lock);
            return mal_post_error(pDevice, MAL_LOG_LEVEL_ERROR, "mal_device_stop() called for a device that's already stopped.", MAL_DEVICE_ALREADY_STOPPED);
        }

        // The device needs to be in a started state. If it's not, we just let the caller know the device is busy.
        if (mal_device__get_state(pDevice) != MAL_STATE_STARTED) {
            mal_mutex_unlock(&pDevice->lock);
            return mal_post_error(pDevice, MAL_LOG_LEVEL_ERROR, "mal_device_stop() called while another thread is in the process of starting it.", MAL_DEVICE_BUSY);
        }

        mal_device__set_state(pDevice, MAL_STATE_STOPPING);

        // There's no need to wake up the thread like we do when starting.

        // Asynchronous backends need to be handled differently.
        if (mal_context_is_backend_asynchronous(pDevice->pContext)) {
            result = pDevice->pContext->onDeviceStop(pDevice);
        } else {
            // Synchronous backends.

            // When we get here the worker thread is likely in a wait state while waiting for the backend device to deliver or request
            // audio data. We need to force these to return as quickly as possible.
            pDevice->pContext->onDeviceBreakMainLoop(pDevice);

            // We need to wait for the worker thread to become available for work before returning. Note that the worker thread will be
            // the one who puts the device into the stopped state. Don't call mal_device__set_state() here.
            mal_event_wait(&pDevice->stopEvent);
            result = MAL_SUCCESS;
        }
    }
    mal_mutex_unlock(&pDevice->lock);

    return result;
}

mal_bool32 mal_device_is_started(mal_device* pDevice)
{
    if (pDevice == NULL) return MAL_FALSE;
    return mal_device__get_state(pDevice) == MAL_STATE_STARTED;
}

mal_uint32 mal_device_get_buffer_size_in_bytes(mal_device* pDevice)
{
    if (pDevice == NULL) return 0;
    return pDevice->bufferSizeInFrames * pDevice->channels * mal_get_bytes_per_sample(pDevice->format);
}

mal_context_config mal_context_config_init(mal_log_proc onLog)
{
    mal_context_config config;
    mal_zero_object(&config);

    config.onLog = onLog;

    return config;
}


mal_device_config mal_device_config_init_default()
{
    mal_device_config config;
    mal_zero_object(&config);

    return config;
}

mal_device_config mal_device_config_init_default_capture(mal_recv_proc onRecvCallback)
{
    mal_device_config config = mal_device_config_init_default();
    config.onRecvCallback = onRecvCallback;

    return config;
}

mal_device_config mal_device_config_init_default_playback(mal_send_proc onSendCallback)
{
    mal_device_config config = mal_device_config_init_default();
    config.onSendCallback = onSendCallback;

    return config;
}


mal_device_config mal_device_config_init_ex(mal_format format, mal_uint32 channels, mal_uint32 sampleRate, mal_channel channelMap[MAL_MAX_CHANNELS], mal_recv_proc onRecvCallback, mal_send_proc onSendCallback)
{
    mal_device_config config = mal_device_config_init_default();

    config.format = format;
    config.channels = channels;
    config.sampleRate = sampleRate;
    config.onRecvCallback = onRecvCallback;
    config.onSendCallback = onSendCallback;

    if (channels > 0) {
        if (channelMap == NULL) {
            if (channels > 8) {
                mal_zero_memory(config.channelMap, sizeof(mal_channel)*MAL_MAX_CHANNELS);
            } else {
                mal_get_standard_channel_map(mal_standard_channel_map_default, channels, config.channelMap);
            }
        } else {
            mal_copy_memory(config.channelMap, channelMap, sizeof(config.channelMap));
        }
    } else {
        mal_zero_memory(config.channelMap, sizeof(mal_channel)*MAL_MAX_CHANNELS);
    }

    return config;
}
#endif  // MAL_NO_DEVICE_IO


void mal_get_standard_channel_map_microsoft(mal_uint32 channels, mal_channel channelMap[MAL_MAX_CHANNELS])
{
    // Based off the speaker configurations mentioned here: https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/content/ksmedia/ns-ksmedia-ksaudio_channel_config
    switch (channels)
    {
        case 1:
        {
            channelMap[0] = MAL_CHANNEL_MONO;
        } break;

        case 2:
        {
            channelMap[0] = MAL_CHANNEL_FRONT_LEFT;
            channelMap[1] = MAL_CHANNEL_FRONT_RIGHT;
        } break;

        case 3: // Not defined, but best guess.
        {
            channelMap[0] = MAL_CHANNEL_FRONT_LEFT;
            channelMap[1] = MAL_CHANNEL_FRONT_RIGHT;
            channelMap[2] = MAL_CHANNEL_FRONT_CENTER;
        } break;

        case 4:
        {
#ifndef MAL_USE_QUAD_MICROSOFT_CHANNEL_MAP
            // Surround. Using the Surround profile has the advantage of the 3rd channel (MAL_CHANNEL_FRONT_CENTER) mapping nicely
            // with higher channel counts.
            channelMap[0] = MAL_CHANNEL_FRONT_LEFT;
            channelMap[1] = MAL_CHANNEL_FRONT_RIGHT;
            channelMap[2] = MAL_CHANNEL_FRONT_CENTER;
            channelMap[3] = MAL_CHANNEL_BACK_CENTER;
#else
            // Quad.
            channelMap[0] = MAL_CHANNEL_FRONT_LEFT;
            channelMap[1] = MAL_CHANNEL_FRONT_RIGHT;
            channelMap[2] = MAL_CHANNEL_BACK_LEFT;
            channelMap[3] = MAL_CHANNEL_BACK_RIGHT;
#endif
        } break;

        case 5: // Not defined, but best guess.
        {
            channelMap[0] = MAL_CHANNEL_FRONT_LEFT;
            channelMap[1] = MAL_CHANNEL_FRONT_RIGHT;
            channelMap[2] = MAL_CHANNEL_FRONT_CENTER;
            channelMap[3] = MAL_CHANNEL_BACK_LEFT;
            channelMap[4] = MAL_CHANNEL_BACK_RIGHT;
        } break;

        case 6:
        {
            channelMap[0] = MAL_CHANNEL_FRONT_LEFT;
            channelMap[1] = MAL_CHANNEL_FRONT_RIGHT;
            channelMap[2] = MAL_CHANNEL_FRONT_CENTER;
            channelMap[3] = MAL_CHANNEL_LFE;
            channelMap[4] = MAL_CHANNEL_SIDE_LEFT;
            channelMap[5] = MAL_CHANNEL_SIDE_RIGHT;
        } break;

        case 7: // Not defined, but best guess.
        {
            channelMap[0] = MAL_CHANNEL_FRONT_LEFT;
            channelMap[1] = MAL_CHANNEL_FRONT_RIGHT;
            channelMap[2] = MAL_CHANNEL_FRONT_CENTER;
            channelMap[3] = MAL_CHANNEL_LFE;
            channelMap[4] = MAL_CHANNEL_BACK_CENTER;
            channelMap[5] = MAL_CHANNEL_SIDE_LEFT;
            channelMap[6] = MAL_CHANNEL_SIDE_RIGHT;
        } break;

        case 8:
        default:
        {
            channelMap[0] = MAL_CHANNEL_FRONT_LEFT;
            channelMap[1] = MAL_CHANNEL_FRONT_RIGHT;
            channelMap[2] = MAL_CHANNEL_FRONT_CENTER;
            channelMap[3] = MAL_CHANNEL_LFE;
            channelMap[4] = MAL_CHANNEL_BACK_LEFT;
            channelMap[5] = MAL_CHANNEL_BACK_RIGHT;
            channelMap[6] = MAL_CHANNEL_SIDE_LEFT;
            channelMap[7] = MAL_CHANNEL_SIDE_RIGHT;
        } break;
    }

    // Remainder.
    if (channels > 8) {
        for (mal_uint32 iChannel = 8; iChannel < MAL_MAX_CHANNELS; ++iChannel) {
            channelMap[iChannel] = (mal_channel)(MAL_CHANNEL_AUX_0 + (iChannel-8));
        }
    }
}

void mal_get_standard_channel_map_alsa(mal_uint32 channels, mal_channel channelMap[MAL_MAX_CHANNELS])
{
    switch (channels)
    {
        case 1:
        {
            channelMap[0] = MAL_CHANNEL_MONO;
        } break;

        case 2:
        {
            channelMap[0] = MAL_CHANNEL_LEFT;
            channelMap[1] = MAL_CHANNEL_RIGHT;
        } break;

        case 3:
        {
            channelMap[0] = MAL_CHANNEL_FRONT_LEFT;
            channelMap[1] = MAL_CHANNEL_FRONT_RIGHT;
            channelMap[2] = MAL_CHANNEL_FRONT_CENTER;
        } break;

        case 4:
        {
            channelMap[0] = MAL_CHANNEL_FRONT_LEFT;
            channelMap[1] = MAL_CHANNEL_FRONT_RIGHT;
            channelMap[2] = MAL_CHANNEL_BACK_LEFT;
            channelMap[3] = MAL_CHANNEL_BACK_RIGHT;
        } break;

        case 5:
        {
            channelMap[0] = MAL_CHANNEL_FRONT_LEFT;
            channelMap[1] = MAL_CHANNEL_FRONT_RIGHT;
            channelMap[2] = MAL_CHANNEL_BACK_LEFT;
            channelMap[3] = MAL_CHANNEL_BACK_RIGHT;
            channelMap[4] = MAL_CHANNEL_FRONT_CENTER;
        } break;

        case 6:
        {
            channelMap[0] = MAL_CHANNEL_FRONT_LEFT;
            channelMap[1] = MAL_CHANNEL_FRONT_RIGHT;
            channelMap[2] = MAL_CHANNEL_BACK_LEFT;
            channelMap[3] = MAL_CHANNEL_BACK_RIGHT;
            channelMap[4] = MAL_CHANNEL_FRONT_CENTER;
            channelMap[5] = MAL_CHANNEL_LFE;
        } break;

        case 7:
        {
            channelMap[0] = MAL_CHANNEL_FRONT_LEFT;
            channelMap[1] = MAL_CHANNEL_FRONT_RIGHT;
            channelMap[2] = MAL_CHANNEL_BACK_LEFT;
            channelMap[3] = MAL_CHANNEL_BACK_RIGHT;
            channelMap[4] = MAL_CHANNEL_FRONT_CENTER;
            channelMap[5] = MAL_CHANNEL_LFE;
            channelMap[6] = MAL_CHANNEL_BACK_CENTER;
        } break;

        case 8:
        default:
        {
            channelMap[0] = MAL_CHANNEL_FRONT_LEFT;
            channelMap[1] = MAL_CHANNEL_FRONT_RIGHT;
            channelMap[2] = MAL_CHANNEL_BACK_LEFT;
            channelMap[3] = MAL_CHANNEL_BACK_RIGHT;
            channelMap[4] = MAL_CHANNEL_FRONT_CENTER;
            channelMap[5] = MAL_CHANNEL_LFE;
            channelMap[6] = MAL_CHANNEL_SIDE_LEFT;
            channelMap[7] = MAL_CHANNEL_SIDE_RIGHT;
        } break;
    }

    // Remainder.
    if (channels > 8) {
        for (mal_uint32 iChannel = 8; iChannel < MAL_MAX_CHANNELS; ++iChannel) {
            channelMap[iChannel] = (mal_channel)(MAL_CHANNEL_AUX_0 + (iChannel-8));
        }
    }
}

void mal_get_standard_channel_map_rfc3551(mal_uint32 channels, mal_channel channelMap[MAL_MAX_CHANNELS])
{
    switch (channels)
    {
        case 1:
        {
            channelMap[0] = MAL_CHANNEL_MONO;
        } break;

        case 2:
        {
            channelMap[0] = MAL_CHANNEL_LEFT;
            channelMap[1] = MAL_CHANNEL_RIGHT;
        } break;

        case 3:
        {
            channelMap[0] = MAL_CHANNEL_FRONT_LEFT;
            channelMap[1] = MAL_CHANNEL_FRONT_RIGHT;
            channelMap[2] = MAL_CHANNEL_FRONT_CENTER;
        } break;

        case 4:
        {
            channelMap[0] = MAL_CHANNEL_FRONT_LEFT;
            channelMap[1] = MAL_CHANNEL_FRONT_CENTER;
            channelMap[2] = MAL_CHANNEL_FRONT_RIGHT;
            channelMap[3] = MAL_CHANNEL_BACK_CENTER;
        } break;

        case 5:
        {
            channelMap[0] = MAL_CHANNEL_FRONT_LEFT;
            channelMap[1] = MAL_CHANNEL_FRONT_RIGHT;
            channelMap[2] = MAL_CHANNEL_FRONT_CENTER;
            channelMap[3] = MAL_CHANNEL_BACK_LEFT;
            channelMap[4] = MAL_CHANNEL_BACK_RIGHT;
        } break;

        case 6:
        {
            channelMap[0] = MAL_CHANNEL_FRONT_LEFT;
            channelMap[1] = MAL_CHANNEL_SIDE_LEFT;
            channelMap[2] = MAL_CHANNEL_FRONT_CENTER;
            channelMap[3] = MAL_CHANNEL_FRONT_RIGHT;
            channelMap[4] = MAL_CHANNEL_SIDE_RIGHT;
            channelMap[5] = MAL_CHANNEL_BACK_CENTER;
        } break;
    }

    // Remainder.
    if (channels > 8) {
        for (mal_uint32 iChannel = 6; iChannel < MAL_MAX_CHANNELS; ++iChannel) {
            channelMap[iChannel] = (mal_channel)(MAL_CHANNEL_AUX_0 + (iChannel-6));
        }
    }
}

void mal_get_standard_channel_map_flac(mal_uint32 channels, mal_channel channelMap[MAL_MAX_CHANNELS])
{
    switch (channels)
    {
        case 1:
        {
            channelMap[0] = MAL_CHANNEL_MONO;
        } break;

        case 2:
        {
            channelMap[0] = MAL_CHANNEL_LEFT;
            channelMap[1] = MAL_CHANNEL_RIGHT;
        } break;

        case 3:
        {
            channelMap[0] = MAL_CHANNEL_FRONT_LEFT;
            channelMap[1] = MAL_CHANNEL_FRONT_RIGHT;
            channelMap[2] = MAL_CHANNEL_FRONT_CENTER;
        } break;

        case 4:
        {
            channelMap[0] = MAL_CHANNEL_FRONT_LEFT;
            channelMap[1] = MAL_CHANNEL_FRONT_RIGHT;
            channelMap[2] = MAL_CHANNEL_BACK_LEFT;
            channelMap[3] = MAL_CHANNEL_BACK_RIGHT;
        } break;

        case 5:
        {
            channelMap[0] = MAL_CHANNEL_FRONT_LEFT;
            channelMap[1] = MAL_CHANNEL_FRONT_RIGHT;
            channelMap[2] = MAL_CHANNEL_FRONT_CENTER;
            channelMap[3] = MAL_CHANNEL_BACK_LEFT;
            channelMap[4] = MAL_CHANNEL_BACK_RIGHT;
        } break;

        case 6:
        {
            channelMap[0] = MAL_CHANNEL_FRONT_LEFT;
            channelMap[1] = MAL_CHANNEL_FRONT_RIGHT;
            channelMap[2] = MAL_CHANNEL_FRONT_CENTER;
            channelMap[3] = MAL_CHANNEL_LFE;
            channelMap[4] = MAL_CHANNEL_BACK_LEFT;
            channelMap[5] = MAL_CHANNEL_BACK_RIGHT;
        } break;

        case 7:
        {
            channelMap[0] = MAL_CHANNEL_FRONT_LEFT;
            channelMap[1] = MAL_CHANNEL_FRONT_RIGHT;
            channelMap[2] = MAL_CHANNEL_FRONT_CENTER;
            channelMap[3] = MAL_CHANNEL_LFE;
            channelMap[4] = MAL_CHANNEL_BACK_CENTER;
            channelMap[5] = MAL_CHANNEL_SIDE_LEFT;
            channelMap[6] = MAL_CHANNEL_SIDE_RIGHT;
        } break;

        case 8:
        default:
        {
            channelMap[0] = MAL_CHANNEL_FRONT_LEFT;
            channelMap[1] = MAL_CHANNEL_FRONT_RIGHT;
            channelMap[2] = MAL_CHANNEL_FRONT_CENTER;
            channelMap[3] = MAL_CHANNEL_LFE;
            channelMap[4] = MAL_CHANNEL_BACK_LEFT;
            channelMap[5] = MAL_CHANNEL_BACK_RIGHT;
            channelMap[6] = MAL_CHANNEL_SIDE_LEFT;
            channelMap[7] = MAL_CHANNEL_SIDE_RIGHT;
        } break;
    }

    // Remainder.
    if (channels > 8) {
        for (mal_uint32 iChannel = 8; iChannel < MAL_MAX_CHANNELS; ++iChannel) {
            channelMap[iChannel] = (mal_channel)(MAL_CHANNEL_AUX_0 + (iChannel-8));
        }
    }
}

void mal_get_standard_channel_map_vorbis(mal_uint32 channels, mal_channel channelMap[MAL_MAX_CHANNELS])
{
    // In Vorbis' type 0 channel mapping, the first two channels are not always the standard left/right - it
    // will have the center speaker where the right usually goes. Why?!
    switch (channels)
    {
        case 1:
        {
            channelMap[0] = MAL_CHANNEL_MONO;
        } break;

        case 2:
        {
            channelMap[0] = MAL_CHANNEL_LEFT;
            channelMap[1] = MAL_CHANNEL_RIGHT;
        } break;

        case 3:
        {
            channelMap[0] = MAL_CHANNEL_FRONT_LEFT;
            channelMap[1] = MAL_CHANNEL_FRONT_CENTER;
            channelMap[2] = MAL_CHANNEL_FRONT_RIGHT;
        } break;

        case 4:
        {
            channelMap[0] = MAL_CHANNEL_FRONT_LEFT;
            channelMap[1] = MAL_CHANNEL_FRONT_RIGHT;
            channelMap[2] = MAL_CHANNEL_BACK_LEFT;
            channelMap[3] = MAL_CHANNEL_BACK_RIGHT;
        } break;

        case 5:
        {
            channelMap[0] = MAL_CHANNEL_FRONT_LEFT;
            channelMap[1] = MAL_CHANNEL_FRONT_CENTER;
            channelMap[2] = MAL_CHANNEL_FRONT_RIGHT;
            channelMap[3] = MAL_CHANNEL_BACK_LEFT;
            channelMap[4] = MAL_CHANNEL_BACK_RIGHT;
        } break;

        case 6:
        {
            channelMap[0] = MAL_CHANNEL_FRONT_LEFT;
            channelMap[1] = MAL_CHANNEL_FRONT_CENTER;
            channelMap[2] = MAL_CHANNEL_FRONT_RIGHT;
            channelMap[3] = MAL_CHANNEL_BACK_LEFT;
            channelMap[4] = MAL_CHANNEL_BACK_RIGHT;
            channelMap[5] = MAL_CHANNEL_LFE;
        } break;

        case 7:
        {
            channelMap[0] = MAL_CHANNEL_FRONT_LEFT;
            channelMap[1] = MAL_CHANNEL_FRONT_CENTER;
            channelMap[2] = MAL_CHANNEL_FRONT_RIGHT;
            channelMap[3] = MAL_CHANNEL_SIDE_LEFT;
            channelMap[4] = MAL_CHANNEL_SIDE_RIGHT;
            channelMap[5] = MAL_CHANNEL_BACK_CENTER;
            channelMap[6] = MAL_CHANNEL_LFE;
        } break;

        case 8:
        default:
        {
            channelMap[0] = MAL_CHANNEL_FRONT_LEFT;
            channelMap[1] = MAL_CHANNEL_FRONT_CENTER;
            channelMap[2] = MAL_CHANNEL_FRONT_RIGHT;
            channelMap[3] = MAL_CHANNEL_SIDE_LEFT;
            channelMap[4] = MAL_CHANNEL_SIDE_RIGHT;
            channelMap[5] = MAL_CHANNEL_BACK_LEFT;
            channelMap[6] = MAL_CHANNEL_BACK_RIGHT;
            channelMap[7] = MAL_CHANNEL_LFE;
        } break;
    }

    // Remainder.
    if (channels > 8) {
        for (mal_uint32 iChannel = 8; iChannel < MAL_MAX_CHANNELS; ++iChannel) {
            channelMap[iChannel] = (mal_channel)(MAL_CHANNEL_AUX_0 + (iChannel-8));
        }
    }
}

void mal_get_standard_channel_map_sound4(mal_uint32 channels, mal_channel channelMap[MAL_MAX_CHANNELS])
{
    switch (channels)
    {
        case 1:
        {
            channelMap[0] = MAL_CHANNEL_MONO;
        } break;

        case 2:
        {
            channelMap[0] = MAL_CHANNEL_LEFT;
            channelMap[1] = MAL_CHANNEL_RIGHT;
        } break;

        case 3:
        {
            channelMap[0] = MAL_CHANNEL_FRONT_LEFT;
            channelMap[1] = MAL_CHANNEL_FRONT_RIGHT;
            channelMap[2] = MAL_CHANNEL_BACK_CENTER;
        } break;

        case 4:
        {
            channelMap[0] = MAL_CHANNEL_FRONT_LEFT;
            channelMap[1] = MAL_CHANNEL_FRONT_RIGHT;
            channelMap[2] = MAL_CHANNEL_BACK_LEFT;
            channelMap[3] = MAL_CHANNEL_BACK_RIGHT;
        } break;

        case 5:
        {
            channelMap[0] = MAL_CHANNEL_FRONT_LEFT;
            channelMap[1] = MAL_CHANNEL_FRONT_RIGHT;
            channelMap[2] = MAL_CHANNEL_BACK_LEFT;
            channelMap[3] = MAL_CHANNEL_BACK_RIGHT;
            channelMap[4] = MAL_CHANNEL_FRONT_CENTER;
        } break;

        case 6:
        {
            channelMap[0] = MAL_CHANNEL_FRONT_LEFT;
            channelMap[1] = MAL_CHANNEL_FRONT_RIGHT;
            channelMap[2] = MAL_CHANNEL_BACK_LEFT;
            channelMap[3] = MAL_CHANNEL_BACK_RIGHT;
            channelMap[4] = MAL_CHANNEL_FRONT_CENTER;
            channelMap[5] = MAL_CHANNEL_LFE;
        } break;

        case 7:
        {
            channelMap[0] = MAL_CHANNEL_FRONT_LEFT;
            channelMap[1] = MAL_CHANNEL_FRONT_RIGHT;
            channelMap[2] = MAL_CHANNEL_BACK_LEFT;
            channelMap[3] = MAL_CHANNEL_BACK_RIGHT;
            channelMap[4] = MAL_CHANNEL_FRONT_CENTER;
            channelMap[5] = MAL_CHANNEL_BACK_CENTER;
            channelMap[6] = MAL_CHANNEL_LFE;
        } break;

        case 8:
        default:
        {
            channelMap[0] = MAL_CHANNEL_FRONT_LEFT;
            channelMap[1] = MAL_CHANNEL_FRONT_RIGHT;
            channelMap[2] = MAL_CHANNEL_BACK_LEFT;
            channelMap[3] = MAL_CHANNEL_BACK_RIGHT;
            channelMap[4] = MAL_CHANNEL_FRONT_CENTER;
            channelMap[5] = MAL_CHANNEL_LFE;
            channelMap[6] = MAL_CHANNEL_SIDE_LEFT;
            channelMap[7] = MAL_CHANNEL_SIDE_RIGHT;
        } break;
    }

    // Remainder.
    if (channels > 8) {
        for (mal_uint32 iChannel = 8; iChannel < MAL_MAX_CHANNELS; ++iChannel) {
            channelMap[iChannel] = (mal_channel)(MAL_CHANNEL_AUX_0 + (iChannel-8));
        }
    }
}

void mal_get_standard_channel_map_sndio(mal_uint32 channels, mal_channel channelMap[MAL_MAX_CHANNELS])
{
    switch (channels)
    {
        case 1:
        {
            channelMap[0] = MAL_CHANNEL_MONO;
        } break;

        case 2:
        {
            channelMap[0] = MAL_CHANNEL_LEFT;
            channelMap[1] = MAL_CHANNEL_RIGHT;
        } break;

        case 3:
        {
            channelMap[0] = MAL_CHANNEL_FRONT_LEFT;
            channelMap[1] = MAL_CHANNEL_FRONT_RIGHT;
            channelMap[2] = MAL_CHANNEL_FRONT_CENTER;
        } break;

        case 4:
        {
            channelMap[0] = MAL_CHANNEL_FRONT_LEFT;
            channelMap[1] = MAL_CHANNEL_FRONT_RIGHT;
            channelMap[2] = MAL_CHANNEL_BACK_LEFT;
            channelMap[3] = MAL_CHANNEL_BACK_RIGHT;
        } break;

        case 5:
        {
            channelMap[0] = MAL_CHANNEL_FRONT_LEFT;
            channelMap[1] = MAL_CHANNEL_FRONT_RIGHT;
            channelMap[2] = MAL_CHANNEL_BACK_LEFT;
            channelMap[3] = MAL_CHANNEL_BACK_RIGHT;
            channelMap[4] = MAL_CHANNEL_FRONT_CENTER;
        } break;

        case 6:
        default:
        {
            channelMap[0] = MAL_CHANNEL_FRONT_LEFT;
            channelMap[1] = MAL_CHANNEL_FRONT_RIGHT;
            channelMap[2] = MAL_CHANNEL_BACK_LEFT;
            channelMap[3] = MAL_CHANNEL_BACK_RIGHT;
            channelMap[4] = MAL_CHANNEL_FRONT_CENTER;
            channelMap[5] = MAL_CHANNEL_LFE;
        } break;
    }

    // Remainder.
    if (channels > 6) {
        for (mal_uint32 iChannel = 6; iChannel < MAL_MAX_CHANNELS; ++iChannel) {
            channelMap[iChannel] = (mal_channel)(MAL_CHANNEL_AUX_0 + (iChannel-6));
        }
    }
}

void mal_get_standard_channel_map(mal_standard_channel_map standardChannelMap, mal_uint32 channels, mal_channel channelMap[MAL_MAX_CHANNELS])
{
    switch (standardChannelMap)
    {
        case mal_standard_channel_map_alsa:
        {
            mal_get_standard_channel_map_alsa(channels, channelMap);
        } break;

        case mal_standard_channel_map_rfc3551:
        {
            mal_get_standard_channel_map_rfc3551(channels, channelMap);
        } break;

        case mal_standard_channel_map_flac:
        {
            mal_get_standard_channel_map_flac(channels, channelMap);
        } break;

        case mal_standard_channel_map_vorbis:
        {
            mal_get_standard_channel_map_vorbis(channels, channelMap);
        } break;

        case mal_standard_channel_map_sound4:
        {
            mal_get_standard_channel_map_sound4(channels, channelMap);
        } break;
        
        case mal_standard_channel_map_sndio:
        {
            mal_get_standard_channel_map_sndio(channels, channelMap);
        } break;

        case mal_standard_channel_map_microsoft:
        default:
        {
            mal_get_standard_channel_map_microsoft(channels, channelMap);
        } break;
    }
}

void mal_channel_map_copy(mal_channel* pOut, const mal_channel* pIn, mal_uint32 channels)
{
    if (pOut != NULL && pIn != NULL && channels > 0) {
        mal_copy_memory(pOut, pIn, sizeof(*pOut) * channels);
    }
}

mal_bool32 mal_channel_map_valid(mal_uint32 channels, const mal_channel channelMap[MAL_MAX_CHANNELS])
{
    if (channelMap == NULL) {
        return MAL_FALSE;
    }

    // A channel count of 0 is invalid.
    if (channels == 0) {
        return MAL_FALSE;
    }

    // It does not make sense to have a mono channel when there is more than 1 channel.
    if (channels > 1) {
        for (mal_uint32 iChannel = 0; iChannel < channels; ++iChannel) {
            if (channelMap[iChannel] == MAL_CHANNEL_MONO) {
                return MAL_FALSE;
            }
        }
    }

    return MAL_TRUE;
}

mal_bool32 mal_channel_map_equal(mal_uint32 channels, const mal_channel channelMapA[MAL_MAX_CHANNELS], const mal_channel channelMapB[MAL_MAX_CHANNELS])
{
    if (channelMapA == channelMapB) {
        return MAL_FALSE;
    }

    if (channels == 0 || channels > MAL_MAX_CHANNELS) {
        return MAL_FALSE;
    }

    for (mal_uint32 iChannel = 0; iChannel < channels; ++iChannel) {
        if (channelMapA[iChannel] != channelMapB[iChannel]) {
            return MAL_FALSE;
        }
    }

    return MAL_TRUE;
}

mal_bool32 mal_channel_map_blank(mal_uint32 channels, const mal_channel channelMap[MAL_MAX_CHANNELS])
{
    for (mal_uint32 iChannel = 0; iChannel < channels; ++iChannel) {
        if (channelMap[iChannel] != MAL_CHANNEL_NONE) {
            return MAL_FALSE;
        }
    }

    return MAL_TRUE;
}

mal_bool32 mal_channel_map_contains_channel_position(mal_uint32 channels, const mal_channel channelMap[MAL_MAX_CHANNELS], mal_channel channelPosition)
{
    for (mal_uint32 iChannel = 0; iChannel < channels; ++iChannel) {
        if (channelMap[iChannel] == channelPosition) {
            return MAL_TRUE;
        }
    }

    return MAL_FALSE;
}




//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Format Conversion.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//#define MAL_USE_REFERENCE_CONVERSION_APIS   1
//#define MAL_USE_SSE

void mal_copy_memory_64(void* dst, const void* src, mal_uint64 sizeInBytes)
{
#if 0xFFFFFFFFFFFFFFFF <= MAL_SIZE_MAX
    mal_copy_memory(dst, src, (size_t)sizeInBytes);
#else
    while (sizeInBytes > 0) {
        mal_uint64 bytesToCopyNow = sizeInBytes;
        if (bytesToCopyNow > MAL_SIZE_MAX) {
            bytesToCopyNow = MAL_SIZE_MAX;
        }

        mal_copy_memory(dst, src, (size_t)bytesToCopyNow);  // Safe cast to size_t.

        sizeInBytes -= bytesToCopyNow;
        dst = (      void*)((      mal_uint8*)dst + bytesToCopyNow);
        src = (const void*)((const mal_uint8*)src + bytesToCopyNow);
    }
#endif
}

void mal_zero_memory_64(void* dst, mal_uint64 sizeInBytes)
{
#if 0xFFFFFFFFFFFFFFFF <= MAL_SIZE_MAX
    mal_zero_memory(dst, (size_t)sizeInBytes);
#else
    while (sizeInBytes > 0) {
        mal_uint64 bytesToZeroNow = sizeInBytes;
        if (bytesToZeroNow > MAL_SIZE_MAX) {
            bytesToZeroNow = MAL_SIZE_MAX;
        }

        mal_zero_memory(dst, (size_t)bytesToZeroNow);  // Safe cast to size_t.

        sizeInBytes -= bytesToZeroNow;
        dst = (void*)((mal_uint8*)dst + bytesToZeroNow);
    }
#endif
}


// u8
void mal_pcm_u8_to_u8(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    (void)ditherMode;
    mal_copy_memory_64(dst, src, count * sizeof(mal_uint8));
}


void mal_pcm_u8_to_s16__reference(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    (void)ditherMode;

    mal_int16* dst_s16 = (mal_int16*)dst;
    const mal_uint8* src_u8 = (const mal_uint8*)src;

    mal_uint64 i;
    for (i = 0; i < count; i += 1) {
        mal_int16 x = src_u8[i];
        x = x - 128;
        x = x << 8;
        dst_s16[i] = x;
    }
}

void mal_pcm_u8_to_s16__optimized(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_u8_to_s16__reference(dst, src, count, ditherMode);
}

#if defined(MAL_SUPPORT_SSE2)
void mal_pcm_u8_to_s16__sse2(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_u8_to_s16__optimized(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_AVX2)
void mal_pcm_u8_to_s16__avx2(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_u8_to_s16__optimized(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_AVX512)
void mal_pcm_u8_to_s16__avx512(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_u8_to_s16__avx2(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_NEON)
void mal_pcm_u8_to_s16__neon(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_u8_to_s16__optimized(dst, src, count, ditherMode);
}
#endif

void mal_pcm_u8_to_s16(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
#ifdef MAL_USE_REFERENCE_CONVERSION_APIS
    mal_pcm_u8_to_s16__reference(dst, src, count, ditherMode);
#else
    mal_pcm_u8_to_s16__optimized(dst, src, count, ditherMode);
#endif
}


void mal_pcm_u8_to_s24__reference(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    (void)ditherMode;

    mal_uint8* dst_s24 = (mal_uint8*)dst;
    const mal_uint8* src_u8 = (const mal_uint8*)src;

    mal_uint64 i;
    for (i = 0; i < count; i += 1) {
        mal_int16 x = src_u8[i];
        x = x - 128;

        dst_s24[i*3+0] = 0;
        dst_s24[i*3+1] = 0;
        dst_s24[i*3+2] = (mal_uint8)((mal_int8)x);
    }
}

void mal_pcm_u8_to_s24__optimized(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_u8_to_s24__reference(dst, src, count, ditherMode);
}

#if defined(MAL_SUPPORT_SSE2)
void mal_pcm_u8_to_s24__sse2(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_u8_to_s24__optimized(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_AVX2)
void mal_pcm_u8_to_s24__avx2(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_u8_to_s24__optimized(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_AVX512)
void mal_pcm_u8_to_s24__avx512(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_u8_to_s24__avx2(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_NEON)
void mal_pcm_u8_to_s24__neon(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_u8_to_s24__optimized(dst, src, count, ditherMode);
}
#endif

void mal_pcm_u8_to_s24(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
#ifdef MAL_USE_REFERENCE_CONVERSION_APIS
    mal_pcm_u8_to_s24__reference(dst, src, count, ditherMode);
#else
    mal_pcm_u8_to_s24__optimized(dst, src, count, ditherMode);
#endif
}


void mal_pcm_u8_to_s32__reference(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    (void)ditherMode;

    mal_int32* dst_s32 = (mal_int32*)dst;
    const mal_uint8* src_u8 = (const mal_uint8*)src;

    mal_uint64 i;
    for (i = 0; i < count; i += 1) {
        mal_int32 x = src_u8[i];
        x = x - 128;
        x = x << 24;
        dst_s32[i] = x;
    }
}

void mal_pcm_u8_to_s32__optimized(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_u8_to_s32__reference(dst, src, count, ditherMode);
}

#if defined(MAL_SUPPORT_SSE2)
void mal_pcm_u8_to_s32__sse2(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_u8_to_s32__optimized(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_AVX2)
void mal_pcm_u8_to_s32__avx2(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_u8_to_s32__optimized(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_AVX512)
void mal_pcm_u8_to_s32__avx512(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_u8_to_s32__avx2(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_NEON)
void mal_pcm_u8_to_s32__neon(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_u8_to_s32__optimized(dst, src, count, ditherMode);
}
#endif

void mal_pcm_u8_to_s32(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
#ifdef MAL_USE_REFERENCE_CONVERSION_APIS
    mal_pcm_u8_to_s32__reference(dst, src, count, ditherMode);
#else
    mal_pcm_u8_to_s32__optimized(dst, src, count, ditherMode);
#endif
}


void mal_pcm_u8_to_f32__reference(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    (void)ditherMode;

    float* dst_f32 = (float*)dst;
    const mal_uint8* src_u8 = (const mal_uint8*)src;

    mal_uint64 i;
    for (i = 0; i < count; i += 1) {
        float x = (float)src_u8[i];
        x = x * 0.00784313725490196078f;    // 0..255 to 0..2
        x = x - 1;                          // 0..2 to -1..1

        dst_f32[i] = x;
    }
}

void mal_pcm_u8_to_f32__optimized(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_u8_to_f32__reference(dst, src, count, ditherMode);
}

#if defined(MAL_SUPPORT_SSE2)
void mal_pcm_u8_to_f32__sse2(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_u8_to_f32__optimized(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_AVX2)
void mal_pcm_u8_to_f32__avx2(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_u8_to_f32__optimized(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_AVX512)
void mal_pcm_u8_to_f32__avx512(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_u8_to_f32__avx2(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_NEON)
void mal_pcm_u8_to_f32__neon(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_u8_to_f32__optimized(dst, src, count, ditherMode);
}
#endif

void mal_pcm_u8_to_f32(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
#ifdef MAL_USE_REFERENCE_CONVERSION_APIS
    mal_pcm_u8_to_f32__reference(dst, src, count, ditherMode);
#else
    mal_pcm_u8_to_f32__optimized(dst, src, count, ditherMode);
#endif
}



void mal_pcm_interleave_u8__reference(void* dst, const void** src, mal_uint64 frameCount, mal_uint32 channels)
{
    mal_uint8* dst_u8 = (mal_uint8*)dst;
    const mal_uint8** src_u8 = (const mal_uint8**)src;

    mal_uint64 iFrame;
    for (iFrame = 0; iFrame < frameCount; iFrame += 1) {
        mal_uint32 iChannel;
        for (iChannel = 0; iChannel < channels; iChannel += 1) {
            dst_u8[iFrame*channels + iChannel] = src_u8[iChannel][iFrame];
        }
    }
}

void mal_pcm_interleave_u8__optimized(void* dst, const void** src, mal_uint64 frameCount, mal_uint32 channels)
{
    mal_uint8* dst_u8 = (mal_uint8*)dst;
    const mal_uint8** src_u8 = (const mal_uint8**)src;

    if (channels == 1) {
        mal_copy_memory_64(dst, src[0], frameCount * sizeof(mal_uint8));
    } else if (channels == 2) {
        mal_uint64 iFrame;
        for (iFrame = 0; iFrame < frameCount; iFrame += 1) {
            dst_u8[iFrame*2 + 0] = src_u8[0][iFrame];
            dst_u8[iFrame*2 + 1] = src_u8[1][iFrame];
        }
    } else {
        mal_uint64 iFrame;
        for (iFrame = 0; iFrame < frameCount; iFrame += 1) {
            mal_uint32 iChannel;
            for (iChannel = 0; iChannel < channels; iChannel += 1) {
                dst_u8[iFrame*channels + iChannel] = src_u8[iChannel][iFrame];
            }
        }
    }
}

void mal_pcm_interleave_u8(void* dst, const void** src, mal_uint64 frameCount, mal_uint32 channels)
{
#ifdef MAL_USE_REFERENCE_CONVERSION_APIS
    mal_pcm_interleave_u8__reference(dst, src, frameCount, channels);
#else
    mal_pcm_interleave_u8__optimized(dst, src, frameCount, channels);
#endif
}


void mal_pcm_deinterleave_u8__reference(void** dst, const void* src, mal_uint64 frameCount, mal_uint32 channels)
{
    mal_uint8** dst_u8 = (mal_uint8**)dst;
    const mal_uint8* src_u8 = (const mal_uint8*)src;

    mal_uint64 iFrame;
    for (iFrame = 0; iFrame < frameCount; iFrame += 1) {
        mal_uint32 iChannel;
        for (iChannel = 0; iChannel < channels; iChannel += 1) {
            dst_u8[iChannel][iFrame] = src_u8[iFrame*channels + iChannel];
        }
    }
}

void mal_pcm_deinterleave_u8__optimized(void** dst, const void* src, mal_uint64 frameCount, mal_uint32 channels)
{
    mal_pcm_deinterleave_u8__reference(dst, src, frameCount, channels);
}

void mal_pcm_deinterleave_u8(void** dst, const void* src, mal_uint64 frameCount, mal_uint32 channels)
{
#ifdef MAL_USE_REFERENCE_CONVERSION_APIS
    mal_pcm_deinterleave_u8__reference(dst, src, frameCount, channels);
#else
    mal_pcm_deinterleave_u8__optimized(dst, src, frameCount, channels);
#endif
}


// s16
void mal_pcm_s16_to_u8__reference(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_uint8* dst_u8 = (mal_uint8*)dst;
    const mal_int16* src_s16 = (const mal_int16*)src;

    if (ditherMode == mal_dither_mode_none) {
        mal_uint64 i;
        for (i = 0; i < count; i += 1) {
            mal_int16 x = src_s16[i];
            x = x >> 8;
            x = x + 128;
            dst_u8[i] = (mal_uint8)x;
        }
    } else {
        mal_uint64 i;
        for (i = 0; i < count; i += 1) {
            mal_int16 x = src_s16[i];

            // Dither. Don't overflow.
            mal_int32 dither = mal_dither_s32(ditherMode, -0x80, 0x7F);
            if ((x + dither) <= 0x7FFF) {
                x = (mal_int16)(x + dither);
            } else {
                x = 0x7FFF;
            }

            x = x >> 8;
            x = x + 128;
            dst_u8[i] = (mal_uint8)x;
        }
    }
}

void mal_pcm_s16_to_u8__optimized(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s16_to_u8__reference(dst, src, count, ditherMode);
}

#if defined(MAL_SUPPORT_SSE2)
void mal_pcm_s16_to_u8__sse2(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s16_to_u8__optimized(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_AVX2)
void mal_pcm_s16_to_u8__avx2(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s16_to_u8__optimized(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_AVX512)
void mal_pcm_s16_to_u8__avx512(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s16_to_u8__avx2(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_NEON)
void mal_pcm_s16_to_u8__neon(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s16_to_u8__optimized(dst, src, count, ditherMode);
}
#endif

void mal_pcm_s16_to_u8(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
#ifdef MAL_USE_REFERENCE_CONVERSION_APIS
    mal_pcm_s16_to_u8__reference(dst, src, count, ditherMode);
#else
    mal_pcm_s16_to_u8__optimized(dst, src, count, ditherMode);
#endif
}


void mal_pcm_s16_to_s16(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    (void)ditherMode;
    mal_copy_memory_64(dst, src, count * sizeof(mal_int16));
}


void mal_pcm_s16_to_s24__reference(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    (void)ditherMode;

    mal_uint8* dst_s24 = (mal_uint8*)dst;
    const mal_int16* src_s16 = (const mal_int16*)src;

    mal_uint64 i;
    for (i = 0; i < count; i += 1) {
        dst_s24[i*3+0] = 0;
        dst_s24[i*3+1] = (mal_uint8)(src_s16[i] & 0xFF);
        dst_s24[i*3+2] = (mal_uint8)(src_s16[i] >> 8);
    }
}

void mal_pcm_s16_to_s24__optimized(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s16_to_s24__reference(dst, src, count, ditherMode);
}

#if defined(MAL_SUPPORT_SSE2)
void mal_pcm_s16_to_s24__sse2(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s16_to_s24__optimized(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_AVX2)
void mal_pcm_s16_to_s24__avx2(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s16_to_s24__optimized(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_AVX512)
void mal_pcm_s16_to_s24__avx512(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s16_to_s24__avx2(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_NEON)
void mal_pcm_s16_to_s24__neon(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s16_to_s24__optimized(dst, src, count, ditherMode);
}
#endif

void mal_pcm_s16_to_s24(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
#ifdef MAL_USE_REFERENCE_CONVERSION_APIS
    mal_pcm_s16_to_s24__reference(dst, src, count, ditherMode);
#else
    mal_pcm_s16_to_s24__optimized(dst, src, count, ditherMode);
#endif
}


void mal_pcm_s16_to_s32__reference(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    (void)ditherMode;

    mal_int32* dst_s32 = (mal_int32*)dst;
    const mal_int16* src_s16 = (const mal_int16*)src;

    mal_uint64 i;
    for (i = 0; i < count; i += 1) {
        dst_s32[i] = src_s16[i] << 16;
    }
}

void mal_pcm_s16_to_s32__optimized(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s16_to_s32__reference(dst, src, count, ditherMode);
}

#if defined(MAL_SUPPORT_SSE2)
void mal_pcm_s16_to_s32__sse2(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s16_to_s32__optimized(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_AVX2)
void mal_pcm_s16_to_s32__avx2(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s16_to_s32__optimized(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_AVX512)
void mal_pcm_s16_to_s32__avx512(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s16_to_s32__avx2(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_NEON)
void mal_pcm_s16_to_s32__neon(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s16_to_s32__optimized(dst, src, count, ditherMode);
}
#endif

void mal_pcm_s16_to_s32(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
#ifdef MAL_USE_REFERENCE_CONVERSION_APIS
    mal_pcm_s16_to_s32__reference(dst, src, count, ditherMode);
#else
    mal_pcm_s16_to_s32__optimized(dst, src, count, ditherMode);
#endif
}


void mal_pcm_s16_to_f32__reference(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    (void)ditherMode;

    float* dst_f32 = (float*)dst;
    const mal_int16* src_s16 = (const mal_int16*)src;

    mal_uint64 i;
    for (i = 0; i < count; i += 1) {
        float x = (float)src_s16[i];

#if 0
        // The accurate way.
        x = x + 32768.0f;                   // -32768..32767 to 0..65535
        x = x * 0.00003051804379339284f;    // 0..65536 to 0..2
        x = x - 1;                          // 0..2 to -1..1
#else
        // The fast way.
        x = x * 0.000030517578125f;         // -32768..32767 to -1..0.999969482421875
#endif

        dst_f32[i] = x;
    }
}

void mal_pcm_s16_to_f32__optimized(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s16_to_f32__reference(dst, src, count, ditherMode);
}

#if defined(MAL_SUPPORT_SSE2)
void mal_pcm_s16_to_f32__sse2(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s16_to_f32__optimized(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_AVX2)
void mal_pcm_s16_to_f32__avx2(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s16_to_f32__optimized(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_AVX512)
void mal_pcm_s16_to_f32__avx512(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s16_to_f32__avx2(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_NEON)
void mal_pcm_s16_to_f32__neon(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s16_to_f32__optimized(dst, src, count, ditherMode);
}
#endif

void mal_pcm_s16_to_f32(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
#ifdef MAL_USE_REFERENCE_CONVERSION_APIS
    mal_pcm_s16_to_f32__reference(dst, src, count, ditherMode);
#else
    mal_pcm_s16_to_f32__optimized(dst, src, count, ditherMode);
#endif
}


void mal_pcm_interleave_s16__reference(void* dst, const void** src, mal_uint64 frameCount, mal_uint32 channels)
{
    mal_int16* dst_s16 = (mal_int16*)dst;
    const mal_int16** src_s16 = (const mal_int16**)src;

    mal_uint64 iFrame;
    for (iFrame = 0; iFrame < frameCount; iFrame += 1) {
        mal_uint32 iChannel;
        for (iChannel = 0; iChannel < channels; iChannel += 1) {
            dst_s16[iFrame*channels + iChannel] = src_s16[iChannel][iFrame];
        }
    }
}

void mal_pcm_interleave_s16__optimized(void* dst, const void** src, mal_uint64 frameCount, mal_uint32 channels)
{
    mal_pcm_interleave_s16__reference(dst, src, frameCount, channels);
}

void mal_pcm_interleave_s16(void* dst, const void** src, mal_uint64 frameCount, mal_uint32 channels)
{
#ifdef MAL_USE_REFERENCE_CONVERSION_APIS
    mal_pcm_interleave_s16__reference(dst, src, frameCount, channels);
#else
    mal_pcm_interleave_s16__optimized(dst, src, frameCount, channels);
#endif
}


void mal_pcm_deinterleave_s16__reference(void** dst, const void* src, mal_uint64 frameCount, mal_uint32 channels)
{
    mal_int16** dst_s16 = (mal_int16**)dst;
    const mal_int16* src_s16 = (const mal_int16*)src;

    mal_uint64 iFrame;
    for (iFrame = 0; iFrame < frameCount; iFrame += 1) {
        mal_uint32 iChannel;
        for (iChannel = 0; iChannel < channels; iChannel += 1) {
            dst_s16[iChannel][iFrame] = src_s16[iFrame*channels + iChannel];
        }
    }
}

void mal_pcm_deinterleave_s16__optimized(void** dst, const void* src, mal_uint64 frameCount, mal_uint32 channels)
{
    mal_pcm_deinterleave_s16__reference(dst, src, frameCount, channels);
}

void mal_pcm_deinterleave_s16(void** dst, const void* src, mal_uint64 frameCount, mal_uint32 channels)
{
#ifdef MAL_USE_REFERENCE_CONVERSION_APIS
    mal_pcm_deinterleave_s16__reference(dst, src, frameCount, channels);
#else
    mal_pcm_deinterleave_s16__optimized(dst, src, frameCount, channels);
#endif
}


// s24
void mal_pcm_s24_to_u8__reference(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_uint8* dst_u8 = (mal_uint8*)dst;
    const mal_uint8* src_s24 = (const mal_uint8*)src;

    if (ditherMode == mal_dither_mode_none) {
        mal_uint64 i;
        for (i = 0; i < count; i += 1) {
            mal_int8 x = (mal_int8)src_s24[i*3 + 2] + 128;
            dst_u8[i] = (mal_uint8)x;
        }
    } else {
        mal_uint64 i;
        for (i = 0; i < count; i += 1) {
            mal_int32 x = (mal_int32)(((mal_uint32)(src_s24[i*3+0]) << 8) | ((mal_uint32)(src_s24[i*3+1]) << 16) | ((mal_uint32)(src_s24[i*3+2])) << 24);

            // Dither. Don't overflow.
            mal_int32 dither = mal_dither_s32(ditherMode, -0x800000, 0x7FFFFF);
            if ((mal_int64)x + dither <= 0x7FFFFFFF) {
                x = x + dither;
            } else {
                x = 0x7FFFFFFF;
            }
            
            x = x >> 24;
            x = x + 128;
            dst_u8[i] = (mal_uint8)x;
        }
    }
}

void mal_pcm_s24_to_u8__optimized(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s24_to_u8__reference(dst, src, count, ditherMode);
}

#if defined(MAL_SUPPORT_SSE2)
void mal_pcm_s24_to_u8__sse2(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s24_to_u8__optimized(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_AVX2)
void mal_pcm_s24_to_u8__avx2(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s24_to_u8__optimized(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_AVX512)
void mal_pcm_s24_to_u8__avx512(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s24_to_u8__avx2(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_NEON)
void mal_pcm_s24_to_u8__neon(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s24_to_u8__optimized(dst, src, count, ditherMode);
}
#endif

void mal_pcm_s24_to_u8(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
#ifdef MAL_USE_REFERENCE_CONVERSION_APIS
    mal_pcm_s24_to_u8__reference(dst, src, count, ditherMode);
#else
    mal_pcm_s24_to_u8__optimized(dst, src, count, ditherMode);
#endif
}


void mal_pcm_s24_to_s16__reference(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_int16* dst_s16 = (mal_int16*)dst;
    const mal_uint8* src_s24 = (const mal_uint8*)src;

    if (ditherMode == mal_dither_mode_none) {
        mal_uint64 i;
        for (i = 0; i < count; i += 1) {
            mal_uint16 dst_lo = ((mal_uint16)src_s24[i*3 + 1]);
            mal_uint16 dst_hi = ((mal_uint16)src_s24[i*3 + 2]) << 8;
            dst_s16[i] = (mal_int16)dst_lo | dst_hi;
        }
    } else {
        mal_uint64 i;
        for (i = 0; i < count; i += 1) {
            mal_int32 x = (mal_int32)(((mal_uint32)(src_s24[i*3+0]) << 8) | ((mal_uint32)(src_s24[i*3+1]) << 16) | ((mal_uint32)(src_s24[i*3+2])) << 24);

            // Dither. Don't overflow.
            mal_int32 dither = mal_dither_s32(ditherMode, -0x8000, 0x7FFF);
            if ((mal_int64)x + dither <= 0x7FFFFFFF) {
                x = x + dither;
            } else {
                x = 0x7FFFFFFF;
            }

            x = x >> 16;
            dst_s16[i] = (mal_int16)x;
        }
    }
}

void mal_pcm_s24_to_s16__optimized(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s24_to_s16__reference(dst, src, count, ditherMode);
}

#if defined(MAL_SUPPORT_SSE2)
void mal_pcm_s24_to_s16__sse2(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s24_to_s16__optimized(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_AVX2)
void mal_pcm_s24_to_s16__avx2(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s24_to_s16__optimized(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_AVX512)
void mal_pcm_s24_to_s16__avx512(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s24_to_s16__avx2(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_NEON)
void mal_pcm_s24_to_s16__neon(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s24_to_s16__optimized(dst, src, count, ditherMode);
}
#endif

void mal_pcm_s24_to_s16(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
#ifdef MAL_USE_REFERENCE_CONVERSION_APIS
    mal_pcm_s24_to_s16__reference(dst, src, count, ditherMode);
#else
    mal_pcm_s24_to_s16__optimized(dst, src, count, ditherMode);
#endif
}


void mal_pcm_s24_to_s24(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    (void)ditherMode;

    mal_copy_memory_64(dst, src, count * 3);
}


void mal_pcm_s24_to_s32__reference(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    (void)ditherMode;

    mal_int32* dst_s32 = (mal_int32*)dst;
    const mal_uint8* src_s24 = (const mal_uint8*)src;

    mal_uint64 i;
    for (i = 0; i < count; i += 1) {
        dst_s32[i] = (mal_int32)(((mal_uint32)(src_s24[i*3+0]) << 8) | ((mal_uint32)(src_s24[i*3+1]) << 16) | ((mal_uint32)(src_s24[i*3+2])) << 24);
    }
}

void mal_pcm_s24_to_s32__optimized(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s24_to_s32__reference(dst, src, count, ditherMode);
}

#if defined(MAL_SUPPORT_SSE2)
void mal_pcm_s24_to_s32__sse2(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s24_to_s32__optimized(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_AVX2)
void mal_pcm_s24_to_s32__avx2(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s24_to_s32__optimized(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_AVX512)
void mal_pcm_s24_to_s32__avx512(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s24_to_s32__avx2(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_NEON)
void mal_pcm_s24_to_s32__neon(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s24_to_s32__optimized(dst, src, count, ditherMode);
}
#endif

void mal_pcm_s24_to_s32(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
#ifdef MAL_USE_REFERENCE_CONVERSION_APIS
    mal_pcm_s24_to_s32__reference(dst, src, count, ditherMode);
#else
    mal_pcm_s24_to_s32__optimized(dst, src, count, ditherMode);
#endif
}


void mal_pcm_s24_to_f32__reference(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    (void)ditherMode;

    float* dst_f32 = (float*)dst;
    const mal_uint8* src_s24 = (const mal_uint8*)src;

    mal_uint64 i;
    for (i = 0; i < count; i += 1) {
        float x = (float)(((mal_int32)(((mal_uint32)(src_s24[i*3+0]) << 8) | ((mal_uint32)(src_s24[i*3+1]) << 16) | ((mal_uint32)(src_s24[i*3+2])) << 24)) >> 8);

#if 0
        // The accurate way.
        x = x + 8388608.0f;                 // -8388608..8388607 to 0..16777215
        x = x * 0.00000011920929665621f;    // 0..16777215 to 0..2
        x = x - 1;                          // 0..2 to -1..1
#else
        // The fast way.
        x = x * 0.00000011920928955078125f; // -8388608..8388607 to -1..0.999969482421875
#endif

        dst_f32[i] = x;
    }
}

void mal_pcm_s24_to_f32__optimized(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s24_to_f32__reference(dst, src, count, ditherMode);
}

#if defined(MAL_SUPPORT_SSE2)
void mal_pcm_s24_to_f32__sse2(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s24_to_f32__optimized(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_AVX2)
void mal_pcm_s24_to_f32__avx2(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s24_to_f32__optimized(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_AVX512)
void mal_pcm_s24_to_f32__avx512(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s24_to_f32__avx2(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_NEON)
void mal_pcm_s24_to_f32__neon(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s24_to_f32__optimized(dst, src, count, ditherMode);
}
#endif

void mal_pcm_s24_to_f32(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
#ifdef MAL_USE_REFERENCE_CONVERSION_APIS
    mal_pcm_s24_to_f32__reference(dst, src, count, ditherMode);
#else
    mal_pcm_s24_to_f32__optimized(dst, src, count, ditherMode);
#endif
}


void mal_pcm_interleave_s24__reference(void* dst, const void** src, mal_uint64 frameCount, mal_uint32 channels)
{
    mal_uint8* dst8 = (mal_uint8*)dst;
    const mal_uint8** src8 = (const mal_uint8**)src;

    mal_uint64 iFrame;
    for (iFrame = 0; iFrame < frameCount; iFrame += 1) {
        mal_uint32 iChannel;
        for (iChannel = 0; iChannel < channels; iChannel += 1) {
            dst8[iFrame*3*channels + iChannel*3 + 0] = src8[iChannel][iFrame*3 + 0];
            dst8[iFrame*3*channels + iChannel*3 + 1] = src8[iChannel][iFrame*3 + 1];
            dst8[iFrame*3*channels + iChannel*3 + 2] = src8[iChannel][iFrame*3 + 2];
        }
    }
}

void mal_pcm_interleave_s24__optimized(void* dst, const void** src, mal_uint64 frameCount, mal_uint32 channels)
{
    mal_pcm_interleave_s24__reference(dst, src, frameCount, channels);
}

void mal_pcm_interleave_s24(void* dst, const void** src, mal_uint64 frameCount, mal_uint32 channels)
{
#ifdef MAL_USE_REFERENCE_CONVERSION_APIS
    mal_pcm_interleave_s24__reference(dst, src, frameCount, channels);
#else
    mal_pcm_interleave_s24__optimized(dst, src, frameCount, channels);
#endif
}


void mal_pcm_deinterleave_s24__reference(void** dst, const void* src, mal_uint64 frameCount, mal_uint32 channels)
{
    mal_uint8** dst8 = (mal_uint8**)dst;
    const mal_uint8* src8 = (const mal_uint8*)src;

    mal_uint32 iFrame;
    for (iFrame = 0; iFrame < frameCount; iFrame += 1) {
        mal_uint32 iChannel;
        for (iChannel = 0; iChannel < channels; iChannel += 1) {
            dst8[iChannel][iFrame*3 + 0] = src8[iFrame*3*channels + iChannel*3 + 0];
            dst8[iChannel][iFrame*3 + 1] = src8[iFrame*3*channels + iChannel*3 + 1];
            dst8[iChannel][iFrame*3 + 2] = src8[iFrame*3*channels + iChannel*3 + 2];
        }
    }
}

void mal_pcm_deinterleave_s24__optimized(void** dst, const void* src, mal_uint64 frameCount, mal_uint32 channels)
{
    mal_pcm_deinterleave_s24__reference(dst, src, frameCount, channels);
}

void mal_pcm_deinterleave_s24(void** dst, const void* src, mal_uint64 frameCount, mal_uint32 channels)
{
#ifdef MAL_USE_REFERENCE_CONVERSION_APIS
    mal_pcm_deinterleave_s24__reference(dst, src, frameCount, channels);
#else
    mal_pcm_deinterleave_s24__optimized(dst, src, frameCount, channels);
#endif
}



// s32
void mal_pcm_s32_to_u8__reference(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_uint8* dst_u8 = (mal_uint8*)dst;
    const mal_int32* src_s32 = (const mal_int32*)src;

    if (ditherMode == mal_dither_mode_none) {
        mal_uint64 i;
        for (i = 0; i < count; i += 1) {
            mal_int32 x = src_s32[i];
            x = x >> 24;
            x = x + 128;
            dst_u8[i] = (mal_uint8)x;
        }
    } else {
        mal_uint64 i;
        for (i = 0; i < count; i += 1) {
            mal_int32 x = src_s32[i];

            // Dither. Don't overflow.
            mal_int32 dither = mal_dither_s32(ditherMode, -0x800000, 0x7FFFFF);
            if ((mal_int64)x + dither <= 0x7FFFFFFF) {
                x = x + dither;
            } else {
                x = 0x7FFFFFFF;
            }
            
            x = x >> 24;
            x = x + 128;
            dst_u8[i] = (mal_uint8)x;
        }
    }
}

void mal_pcm_s32_to_u8__optimized(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s32_to_u8__reference(dst, src, count, ditherMode);
}

#if defined(MAL_SUPPORT_SSE2)
void mal_pcm_s32_to_u8__sse2(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s32_to_u8__optimized(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_AVX2)
void mal_pcm_s32_to_u8__avx2(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s32_to_u8__optimized(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_AVX512)
void mal_pcm_s32_to_u8__avx512(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s32_to_u8__avx2(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_NEON)
void mal_pcm_s32_to_u8__neon(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s32_to_u8__optimized(dst, src, count, ditherMode);
}
#endif

void mal_pcm_s32_to_u8(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
#ifdef MAL_USE_REFERENCE_CONVERSION_APIS
    mal_pcm_s32_to_u8__reference(dst, src, count, ditherMode);
#else
    mal_pcm_s32_to_u8__optimized(dst, src, count, ditherMode);
#endif
}


void mal_pcm_s32_to_s16__reference(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_int16* dst_s16 = (mal_int16*)dst;
    const mal_int32* src_s32 = (const mal_int32*)src;

    if (ditherMode == mal_dither_mode_none) {
        mal_uint64 i;
        for (i = 0; i < count; i += 1) {
            mal_int32 x = src_s32[i];
            x = x >> 16;
            dst_s16[i] = (mal_int16)x;
        }
    } else {
        mal_uint64 i;
        for (i = 0; i < count; i += 1) {
            mal_int32 x = src_s32[i];

            // Dither. Don't overflow.
            mal_int32 dither = mal_dither_s32(ditherMode, -0x8000, 0x7FFF);
            if ((mal_int64)x + dither <= 0x7FFFFFFF) {
                x = x + dither;
            } else {
                x = 0x7FFFFFFF;
            }
            
            x = x >> 16;
            dst_s16[i] = (mal_int16)x;
        }
    }
}

void mal_pcm_s32_to_s16__optimized(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s32_to_s16__reference(dst, src, count, ditherMode);
}

#if defined(MAL_SUPPORT_SSE2)
void mal_pcm_s32_to_s16__sse2(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s32_to_s16__optimized(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_AVX2)
void mal_pcm_s32_to_s16__avx2(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s32_to_s16__optimized(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_AVX512)
void mal_pcm_s32_to_s16__avx512(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s32_to_s16__avx2(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_NEON)
void mal_pcm_s32_to_s16__neon(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s32_to_s16__optimized(dst, src, count, ditherMode);
}
#endif

void mal_pcm_s32_to_s16(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
#ifdef MAL_USE_REFERENCE_CONVERSION_APIS
    mal_pcm_s32_to_s16__reference(dst, src, count, ditherMode);
#else
    mal_pcm_s32_to_s16__optimized(dst, src, count, ditherMode);
#endif
}


void mal_pcm_s32_to_s24__reference(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    (void)ditherMode;   // No dithering for s32 -> s24.

    mal_uint8* dst_s24 = (mal_uint8*)dst;
    const mal_int32* src_s32 = (const mal_int32*)src;

    mal_uint64 i;
    for (i = 0; i < count; i += 1) {
        mal_uint32 x = (mal_uint32)src_s32[i];
        dst_s24[i*3+0] = (mal_uint8)((x & 0x0000FF00) >>  8);
        dst_s24[i*3+1] = (mal_uint8)((x & 0x00FF0000) >> 16);
        dst_s24[i*3+2] = (mal_uint8)((x & 0xFF000000) >> 24);
    }
}

void mal_pcm_s32_to_s24__optimized(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s32_to_s24__reference(dst, src, count, ditherMode);
}

#if defined(MAL_SUPPORT_SSE2)
void mal_pcm_s32_to_s24__sse2(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s32_to_s24__optimized(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_AVX2)
void mal_pcm_s32_to_s24__avx2(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s32_to_s24__optimized(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_AVX512)
void mal_pcm_s32_to_s24__avx512(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s32_to_s24__avx2(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_NEON)
void mal_pcm_s32_to_s24__neon(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s32_to_s24__optimized(dst, src, count, ditherMode);
}
#endif

void mal_pcm_s32_to_s24(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
#ifdef MAL_USE_REFERENCE_CONVERSION_APIS
    mal_pcm_s32_to_s24__reference(dst, src, count, ditherMode);
#else
    mal_pcm_s32_to_s24__optimized(dst, src, count, ditherMode);
#endif
}


void mal_pcm_s32_to_s32(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    (void)ditherMode;

    mal_copy_memory_64(dst, src, count * sizeof(mal_int32));
}


void mal_pcm_s32_to_f32__reference(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    (void)ditherMode;   // No dithering for s32 -> f32.

    float* dst_f32 = (float*)dst;
    const mal_int32* src_s32 = (const mal_int32*)src;

    mal_uint64 i;
    for (i = 0; i < count; i += 1) {
        double x = src_s32[i];

#if 0
        x = x + 2147483648.0;
        x = x * 0.0000000004656612873077392578125;
        x = x - 1;
#else
        x = x / 2147483648.0;
#endif

        dst_f32[i] = (float)x;
    }
}

void mal_pcm_s32_to_f32__optimized(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s32_to_f32__reference(dst, src, count, ditherMode);
}

#if defined(MAL_SUPPORT_SSE2)
void mal_pcm_s32_to_f32__sse2(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s32_to_f32__optimized(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_AVX2)
void mal_pcm_s32_to_f32__avx2(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s32_to_f32__optimized(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_AVX512)
void mal_pcm_s32_to_f32__avx512(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s32_to_f32__avx2(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_NEON)
void mal_pcm_s32_to_f32__neon(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_s32_to_f32__optimized(dst, src, count, ditherMode);
}
#endif

void mal_pcm_s32_to_f32(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
#ifdef MAL_USE_REFERENCE_CONVERSION_APIS
    mal_pcm_s32_to_f32__reference(dst, src, count, ditherMode);
#else
    mal_pcm_s32_to_f32__optimized(dst, src, count, ditherMode);
#endif
}


void mal_pcm_interleave_s32__reference(void* dst, const void** src, mal_uint64 frameCount, mal_uint32 channels)
{
    mal_int32* dst_s32 = (mal_int32*)dst;
    const mal_int32** src_s32 = (const mal_int32**)src;

    mal_uint64 iFrame;
    for (iFrame = 0; iFrame < frameCount; iFrame += 1) {
        mal_uint32 iChannel;
        for (iChannel = 0; iChannel < channels; iChannel += 1) {
            dst_s32[iFrame*channels + iChannel] = src_s32[iChannel][iFrame];
        }
    }
}

void mal_pcm_interleave_s32__optimized(void* dst, const void** src, mal_uint64 frameCount, mal_uint32 channels)
{
    mal_pcm_interleave_s32__reference(dst, src, frameCount, channels);
}

void mal_pcm_interleave_s32(void* dst, const void** src, mal_uint64 frameCount, mal_uint32 channels)
{
#ifdef MAL_USE_REFERENCE_CONVERSION_APIS
    mal_pcm_interleave_s32__reference(dst, src, frameCount, channels);
#else
    mal_pcm_interleave_s32__optimized(dst, src, frameCount, channels);
#endif
}


void mal_pcm_deinterleave_s32__reference(void** dst, const void* src, mal_uint64 frameCount, mal_uint32 channels)
{
    mal_int32** dst_s32 = (mal_int32**)dst;
    const mal_int32* src_s32 = (const mal_int32*)src;

    mal_uint64 iFrame;
    for (iFrame = 0; iFrame < frameCount; iFrame += 1) {
        mal_uint32 iChannel;
        for (iChannel = 0; iChannel < channels; iChannel += 1) {
            dst_s32[iChannel][iFrame] = src_s32[iFrame*channels + iChannel];
        }
    }
}

void mal_pcm_deinterleave_s32__optimized(void** dst, const void* src, mal_uint64 frameCount, mal_uint32 channels)
{
    mal_pcm_deinterleave_s32__reference(dst, src, frameCount, channels);
}

void mal_pcm_deinterleave_s32(void** dst, const void* src, mal_uint64 frameCount, mal_uint32 channels)
{
#ifdef MAL_USE_REFERENCE_CONVERSION_APIS
    mal_pcm_deinterleave_s32__reference(dst, src, frameCount, channels);
#else
    mal_pcm_deinterleave_s32__optimized(dst, src, frameCount, channels);
#endif
}


// f32
void mal_pcm_f32_to_u8__reference(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_uint8* dst_u8 = (mal_uint8*)dst;
    const float* src_f32 = (const float*)src;

    float ditherMin = 0;
    float ditherMax = 0;
    if (ditherMode != mal_dither_mode_none) {
        ditherMin = 1.0f / -128;
        ditherMax = 1.0f /  127;
    }

    mal_uint64 i;
    for (i = 0; i < count; i += 1) {
        float x = src_f32[i];
        x = x + mal_dither_f32(ditherMode, ditherMin, ditherMax);
        x = ((x < -1) ? -1 : ((x > 1) ? 1 : x));    // clip
        x = x + 1;                                  // -1..1 to 0..2
        x = x * 127.5f;                             // 0..2 to 0..255

        dst_u8[i] = (mal_uint8)x;
    }
}

void mal_pcm_f32_to_u8__optimized(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_f32_to_u8__reference(dst, src, count, ditherMode);
}

#if defined(MAL_SUPPORT_SSE2)
void mal_pcm_f32_to_u8__sse2(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_f32_to_u8__optimized(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_AVX2)
void mal_pcm_f32_to_u8__avx2(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_f32_to_u8__optimized(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_AVX512)
void mal_pcm_f32_to_u8__avx512(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_f32_to_u8__avx2(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_NEON)
void mal_pcm_f32_to_u8__neon(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_f32_to_u8__optimized(dst, src, count, ditherMode);
}
#endif

void mal_pcm_f32_to_u8(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
#ifdef MAL_USE_REFERENCE_CONVERSION_APIS
    mal_pcm_f32_to_u8__reference(dst, src, count, ditherMode);
#else
    mal_pcm_f32_to_u8__optimized(dst, src, count, ditherMode);
#endif
}


void mal_pcm_f32_to_s16__reference(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_int16* dst_s16 = (mal_int16*)dst;
    const float* src_f32 = (const float*)src;

    float ditherMin = 0;
    float ditherMax = 0;
    if (ditherMode != mal_dither_mode_none) {
        ditherMin = 1.0f / -32768;
        ditherMax = 1.0f /  32767;
    }

    mal_uint64 i;
    for (i = 0; i < count; i += 1) {
        float x = src_f32[i];
        x = x + mal_dither_f32(ditherMode, ditherMin, ditherMax);
        x = ((x < -1) ? -1 : ((x > 1) ? 1 : x));    // clip

#if 0
        // The accurate way.
        x = x + 1;                                  // -1..1 to 0..2
        x = x * 32767.5f;                           // 0..2 to 0..65535
        x = x - 32768.0f;                           // 0...65535 to -32768..32767
#else
        // The fast way.
        x = x * 32767.0f;                           // -1..1 to -32767..32767
#endif

        dst_s16[i] = (mal_int16)x;
    }
}

void mal_pcm_f32_to_s16__optimized(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_int16* dst_s16 = (mal_int16*)dst;
    const float* src_f32 = (const float*)src;

    float ditherMin = 0;
    float ditherMax = 0;
    if (ditherMode != mal_dither_mode_none) {
        ditherMin = 1.0f / -32768;
        ditherMax = 1.0f /  32767;
    }

    mal_uint64 i = 0;

    // Unrolled.
    mal_uint64 count4 = count >> 2;
    for (mal_uint64 i4 = 0; i4 < count4; i4 += 1) {
        float d0 = mal_dither_f32(ditherMode, ditherMin, ditherMax);
        float d1 = mal_dither_f32(ditherMode, ditherMin, ditherMax);
        float d2 = mal_dither_f32(ditherMode, ditherMin, ditherMax);
        float d3 = mal_dither_f32(ditherMode, ditherMin, ditherMax);
        
        float x0 = src_f32[i+0];
        float x1 = src_f32[i+1];
        float x2 = src_f32[i+2];
        float x3 = src_f32[i+3];

        x0 = x0 + d0;
        x1 = x1 + d1;
        x2 = x2 + d2;
        x3 = x3 + d3;

        x0 = ((x0 < -1) ? -1 : ((x0 > 1) ? 1 : x0));
        x1 = ((x1 < -1) ? -1 : ((x1 > 1) ? 1 : x1));
        x2 = ((x2 < -1) ? -1 : ((x2 > 1) ? 1 : x2));
        x3 = ((x3 < -1) ? -1 : ((x3 > 1) ? 1 : x3));

        x0 = x0 * 32767.0f;
        x1 = x1 * 32767.0f;
        x2 = x2 * 32767.0f;
        x3 = x3 * 32767.0f;

        dst_s16[i+0] = (mal_int16)x0;
        dst_s16[i+1] = (mal_int16)x1;
        dst_s16[i+2] = (mal_int16)x2;
        dst_s16[i+3] = (mal_int16)x3;

        i += 4;
    }

    // Leftover.
    for (; i < count; i += 1) {
        float x = src_f32[i];
        x = x + mal_dither_f32(ditherMode, ditherMin, ditherMax);
        x = ((x < -1) ? -1 : ((x > 1) ? 1 : x));    // clip
        x = x * 32767.0f;                           // -1..1 to -32767..32767

        dst_s16[i] = (mal_int16)x;
    }
}

#if defined(MAL_SUPPORT_SSE2)
void mal_pcm_f32_to_s16__sse2(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    // Both the input and output buffers need to be aligned to 16 bytes.
    if ((((mal_uintptr)dst & 15) != 0) || (((mal_uintptr)src & 15) != 0)) {
        mal_pcm_f32_to_s16__optimized(dst, src, count, ditherMode);
        return;
    }

    mal_int16* dst_s16 = (mal_int16*)dst;
    const float* src_f32 = (const float*)src;

    float ditherMin = 0;
    float ditherMax = 0;
    if (ditherMode != mal_dither_mode_none) {
        ditherMin = 1.0f / -32768;
        ditherMax = 1.0f /  32767;
    }

    mal_uint64 i = 0;

    // SSE2. SSE allows us to output 8 s16's at a time which means our loop is unrolled 8 times.
    mal_uint64 count8 = count >> 3;
    for (mal_uint64 i8 = 0; i8 < count8; i8 += 1) {
        __m128 d0;
        __m128 d1;
        if (ditherMode == mal_dither_mode_none) {
            d0 = _mm_set1_ps(0);
            d1 = _mm_set1_ps(0);
        } else if (ditherMode == mal_dither_mode_rectangle) {
            d0 = _mm_set_ps(
                mal_dither_f32_rectangle(ditherMin, ditherMax),
                mal_dither_f32_rectangle(ditherMin, ditherMax),
                mal_dither_f32_rectangle(ditherMin, ditherMax),
                mal_dither_f32_rectangle(ditherMin, ditherMax)
            );
            d1 = _mm_set_ps(
                mal_dither_f32_rectangle(ditherMin, ditherMax),
                mal_dither_f32_rectangle(ditherMin, ditherMax),
                mal_dither_f32_rectangle(ditherMin, ditherMax),
                mal_dither_f32_rectangle(ditherMin, ditherMax)
            );
        } else {
            d0 = _mm_set_ps(
                mal_dither_f32_triangle(ditherMin, ditherMax),
                mal_dither_f32_triangle(ditherMin, ditherMax),
                mal_dither_f32_triangle(ditherMin, ditherMax),
                mal_dither_f32_triangle(ditherMin, ditherMax)
            );
            d1 = _mm_set_ps(
                mal_dither_f32_triangle(ditherMin, ditherMax),
                mal_dither_f32_triangle(ditherMin, ditherMax),
                mal_dither_f32_triangle(ditherMin, ditherMax),
                mal_dither_f32_triangle(ditherMin, ditherMax)
            );
        }

        __m128 x0 = *((__m128*)(src_f32 + i) + 0);
        __m128 x1 = *((__m128*)(src_f32 + i) + 1);

        x0 = _mm_add_ps(x0, d0);
        x1 = _mm_add_ps(x1, d1);

        x0 = _mm_mul_ps(x0, _mm_set1_ps(32767.0f));
        x1 = _mm_mul_ps(x1, _mm_set1_ps(32767.0f));

        _mm_stream_si128(((__m128i*)(dst_s16 + i)), _mm_packs_epi32(_mm_cvttps_epi32(x0), _mm_cvttps_epi32(x1)));
        
        i += 8;
    }


    // Leftover.
    for (; i < count; i += 1) {
        float x = src_f32[i];
        x = x + mal_dither_f32(ditherMode, ditherMin, ditherMax);
        x = ((x < -1) ? -1 : ((x > 1) ? 1 : x));    // clip
        x = x * 32767.0f;                           // -1..1 to -32767..32767

        dst_s16[i] = (mal_int16)x;
    }
}
#endif
#if defined(MAL_SUPPORT_AVX2)
void mal_pcm_f32_to_s16__avx2(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    // Both the input and output buffers need to be aligned to 32 bytes.
    if ((((mal_uintptr)dst & 31) != 0) || (((mal_uintptr)src & 31) != 0)) {
        mal_pcm_f32_to_s16__optimized(dst, src, count, ditherMode);
        return;
    }

    mal_int16* dst_s16 = (mal_int16*)dst;
    const float* src_f32 = (const float*)src;

    float ditherMin = 0;
    float ditherMax = 0;
    if (ditherMode != mal_dither_mode_none) {
        ditherMin = 1.0f / -32768;
        ditherMax = 1.0f /  32767;
    }

    mal_uint64 i = 0;

    // AVX2. AVX2 allows us to output 16 s16's at a time which means our loop is unrolled 16 times.
    mal_uint64 count16 = count >> 4;
    for (mal_uint64 i16 = 0; i16 < count16; i16 += 1) {
        __m256 d0;
        __m256 d1;
        if (ditherMode == mal_dither_mode_none) {
            d0 = _mm256_set1_ps(0);
            d1 = _mm256_set1_ps(0);
        } else if (ditherMode == mal_dither_mode_rectangle) {
            d0 = _mm256_set_ps(
                mal_dither_f32_rectangle(ditherMin, ditherMax),
                mal_dither_f32_rectangle(ditherMin, ditherMax),
                mal_dither_f32_rectangle(ditherMin, ditherMax),
                mal_dither_f32_rectangle(ditherMin, ditherMax),
                mal_dither_f32_rectangle(ditherMin, ditherMax),
                mal_dither_f32_rectangle(ditherMin, ditherMax),
                mal_dither_f32_rectangle(ditherMin, ditherMax),
                mal_dither_f32_rectangle(ditherMin, ditherMax)
            );
            d1 = _mm256_set_ps(
                mal_dither_f32_rectangle(ditherMin, ditherMax),
                mal_dither_f32_rectangle(ditherMin, ditherMax),
                mal_dither_f32_rectangle(ditherMin, ditherMax),
                mal_dither_f32_rectangle(ditherMin, ditherMax),
                mal_dither_f32_rectangle(ditherMin, ditherMax),
                mal_dither_f32_rectangle(ditherMin, ditherMax),
                mal_dither_f32_rectangle(ditherMin, ditherMax),
                mal_dither_f32_rectangle(ditherMin, ditherMax)
            );
        } else {
            d0 = _mm256_set_ps(
                mal_dither_f32_triangle(ditherMin, ditherMax),
                mal_dither_f32_triangle(ditherMin, ditherMax),
                mal_dither_f32_triangle(ditherMin, ditherMax),
                mal_dither_f32_triangle(ditherMin, ditherMax),
                mal_dither_f32_triangle(ditherMin, ditherMax),
                mal_dither_f32_triangle(ditherMin, ditherMax),
                mal_dither_f32_triangle(ditherMin, ditherMax),
                mal_dither_f32_triangle(ditherMin, ditherMax)
            );
            d1 = _mm256_set_ps(
                mal_dither_f32_triangle(ditherMin, ditherMax),
                mal_dither_f32_triangle(ditherMin, ditherMax),
                mal_dither_f32_triangle(ditherMin, ditherMax),
                mal_dither_f32_triangle(ditherMin, ditherMax),
                mal_dither_f32_triangle(ditherMin, ditherMax),
                mal_dither_f32_triangle(ditherMin, ditherMax),
                mal_dither_f32_triangle(ditherMin, ditherMax),
                mal_dither_f32_triangle(ditherMin, ditherMax)
            );
        }

        __m256 x0 = *((__m256*)(src_f32 + i) + 0);
        __m256 x1 = *((__m256*)(src_f32 + i) + 1);

        x0 = _mm256_add_ps(x0, d0);
        x1 = _mm256_add_ps(x1, d1);

        x0 = _mm256_mul_ps(x0, _mm256_set1_ps(32767.0f));
        x1 = _mm256_mul_ps(x1, _mm256_set1_ps(32767.0f));

        // Computing the final result is a little more complicated for AVX2 than SSE2.
        __m256i i0 = _mm256_cvttps_epi32(x0);
        __m256i i1 = _mm256_cvttps_epi32(x1);
        __m256i p0 = _mm256_permute2x128_si256(i0, i1, 0 | 32);
        __m256i p1 = _mm256_permute2x128_si256(i0, i1, 1 | 48);
        __m256i r  = _mm256_packs_epi32(p0, p1);

        _mm256_stream_si256(((__m256i*)(dst_s16 + i)), r);

        i += 16;
    }


    // Leftover.
    for (; i < count; i += 1) {
        float x = src_f32[i];
        x = x + mal_dither_f32(ditherMode, ditherMin, ditherMax);
        x = ((x < -1) ? -1 : ((x > 1) ? 1 : x));    // clip
        x = x * 32767.0f;                           // -1..1 to -32767..32767

        dst_s16[i] = (mal_int16)x;
    }
}
#endif
#if defined(MAL_SUPPORT_AVX512)
void mal_pcm_f32_to_s16__avx512(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    // TODO: Convert this from AVX to AVX-512.
    mal_pcm_f32_to_s16__avx2(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_NEON)
void mal_pcm_f32_to_s16__neon(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    // Both the input and output buffers need to be aligned to 16 bytes.
    if ((((mal_uintptr)dst & 15) != 0) || (((mal_uintptr)src & 15) != 0)) {
        mal_pcm_f32_to_s16__optimized(dst, src, count, ditherMode);
        return;
    }

    mal_int16* dst_s16 = (mal_int16*)dst;
    const float* src_f32 = (const float*)src;

    float ditherMin = 0;
    float ditherMax = 0;
    if (ditherMode != mal_dither_mode_none) {
        ditherMin = 1.0f / -32768;
        ditherMax = 1.0f /  32767;
    }

    mal_uint64 i = 0;

    // NEON. NEON allows us to output 8 s16's at a time which means our loop is unrolled 8 times.
    mal_uint64 count8 = count >> 3;
    for (mal_uint64 i8 = 0; i8 < count8; i8 += 1) {
        float32x4_t d0;
        float32x4_t d1;
        if (ditherMode == mal_dither_mode_none) {
            d0 = vmovq_n_f32(0);
            d1 = vmovq_n_f32(0);
        } else if (ditherMode == mal_dither_mode_rectangle) {
            float d0v[4];
            d0v[0] = mal_dither_f32_rectangle(ditherMin, ditherMax);
            d0v[1] = mal_dither_f32_rectangle(ditherMin, ditherMax);
            d0v[2] = mal_dither_f32_rectangle(ditherMin, ditherMax);
            d0v[3] = mal_dither_f32_rectangle(ditherMin, ditherMax);
            d0 = vld1q_f32(d0v);

            float d1v[4];
            d1v[0] = mal_dither_f32_rectangle(ditherMin, ditherMax);
            d1v[1] = mal_dither_f32_rectangle(ditherMin, ditherMax);
            d1v[2] = mal_dither_f32_rectangle(ditherMin, ditherMax);
            d1v[3] = mal_dither_f32_rectangle(ditherMin, ditherMax);
            d1 = vld1q_f32(d1v);
        } else {
            float d0v[4];
            d0v[0] = mal_dither_f32_triangle(ditherMin, ditherMax);
            d0v[1] = mal_dither_f32_triangle(ditherMin, ditherMax);
            d0v[2] = mal_dither_f32_triangle(ditherMin, ditherMax);
            d0v[3] = mal_dither_f32_triangle(ditherMin, ditherMax);
            d0 = vld1q_f32(d0v);

            float d1v[4];
            d1v[0] = mal_dither_f32_triangle(ditherMin, ditherMax);
            d1v[1] = mal_dither_f32_triangle(ditherMin, ditherMax);
            d1v[2] = mal_dither_f32_triangle(ditherMin, ditherMax);
            d1v[3] = mal_dither_f32_triangle(ditherMin, ditherMax);
            d1 = vld1q_f32(d1v);
        }

        float32x4_t x0 = *((float32x4_t*)(src_f32 + i) + 0);
        float32x4_t x1 = *((float32x4_t*)(src_f32 + i) + 1);

        x0 = vaddq_f32(x0, d0);
        x1 = vaddq_f32(x1, d1);

        x0 = vmulq_n_f32(x0, 32767.0f);
        x1 = vmulq_n_f32(x1, 32767.0f);

        int32x4_t i0 = vcvtq_s32_f32(x0);
        int32x4_t i1 = vcvtq_s32_f32(x1);
        *((int16x8_t*)(dst_s16 + i)) = vcombine_s16(vqmovn_s32(i0), vqmovn_s32(i1));

        i += 8;
    }


    // Leftover.
    for (; i < count; i += 1) {
        float x = src_f32[i];
        x = x + mal_dither_f32(ditherMode, ditherMin, ditherMax);
        x = ((x < -1) ? -1 : ((x > 1) ? 1 : x));    // clip
        x = x * 32767.0f;                           // -1..1 to -32767..32767

        dst_s16[i] = (mal_int16)x;
    }
}
#endif

void mal_pcm_f32_to_s16(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
#ifdef MAL_USE_REFERENCE_CONVERSION_APIS
    mal_pcm_f32_to_s16__reference(dst, src, count, ditherMode);
#else
    mal_pcm_f32_to_s16__optimized(dst, src, count, ditherMode);
#endif
}


void mal_pcm_f32_to_s24__reference(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    (void)ditherMode;   // No dithering for f32 -> s24.

    mal_uint8* dst_s24 = (mal_uint8*)dst;
    const float* src_f32 = (const float*)src;

    mal_uint64 i;
    for (i = 0; i < count; i += 1) {
        float x = src_f32[i];
        x = ((x < -1) ? -1 : ((x > 1) ? 1 : x));    // clip

#if 0
        // The accurate way.
        x = x + 1;                                  // -1..1 to 0..2
        x = x * 8388607.5f;                         // 0..2 to 0..16777215
        x = x - 8388608.0f;                         // 0..16777215 to -8388608..8388607
#else
        // The fast way.
        x = x * 8388607.0f;                         // -1..1 to -8388607..8388607
#endif

        mal_int32 r = (mal_int32)x;
        dst_s24[(i*3)+0] = (mal_uint8)((r & 0x0000FF) >>  0);
        dst_s24[(i*3)+1] = (mal_uint8)((r & 0x00FF00) >>  8);
        dst_s24[(i*3)+2] = (mal_uint8)((r & 0xFF0000) >> 16);
    }
}

void mal_pcm_f32_to_s24__optimized(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_f32_to_s24__reference(dst, src, count, ditherMode);
}

#if defined(MAL_SUPPORT_SSE2)
void mal_pcm_f32_to_s24__sse2(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_f32_to_s24__optimized(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_AVX2)
void mal_pcm_f32_to_s24__avx2(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_f32_to_s24__optimized(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_AVX512)
void mal_pcm_f32_to_s24__avx512(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_f32_to_s24__avx2(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_NEON)
void mal_pcm_f32_to_s24__neon(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_f32_to_s24__optimized(dst, src, count, ditherMode);
}
#endif

void mal_pcm_f32_to_s24(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
#ifdef MAL_USE_REFERENCE_CONVERSION_APIS
    mal_pcm_f32_to_s24__reference(dst, src, count, ditherMode);
#else
    mal_pcm_f32_to_s24__optimized(dst, src, count, ditherMode);
#endif
}


void mal_pcm_f32_to_s32__reference(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    (void)ditherMode;   // No dithering for f32 -> s32.

    mal_int32* dst_s32 = (mal_int32*)dst;
    const float* src_f32 = (const float*)src;

    mal_uint32 i;
    for (i = 0; i < count; i += 1) {
        double x = src_f32[i];
        x = ((x < -1) ? -1 : ((x > 1) ? 1 : x));    // clip

#if 0
        // The accurate way.
        x = x + 1;                                  // -1..1 to 0..2
        x = x * 2147483647.5;                       // 0..2 to 0..4294967295
        x = x - 2147483648.0;                       // 0...4294967295 to -2147483648..2147483647
#else
        // The fast way.
        x = x * 2147483647.0;                       // -1..1 to -2147483647..2147483647
#endif

        dst_s32[i] = (mal_int32)x;
    }
}

void mal_pcm_f32_to_s32__optimized(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_f32_to_s32__reference(dst, src, count, ditherMode);
}

#if defined(MAL_SUPPORT_SSE2)
void mal_pcm_f32_to_s32__sse2(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_f32_to_s32__optimized(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_AVX2)
void mal_pcm_f32_to_s32__avx2(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_f32_to_s32__optimized(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_AVX512)
void mal_pcm_f32_to_s32__avx512(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_f32_to_s32__avx2(dst, src, count, ditherMode);
}
#endif
#if defined(MAL_SUPPORT_NEON)
void mal_pcm_f32_to_s32__neon(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    mal_pcm_f32_to_s32__optimized(dst, src, count, ditherMode);
}
#endif

void mal_pcm_f32_to_s32(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
#ifdef MAL_USE_REFERENCE_CONVERSION_APIS
    mal_pcm_f32_to_s32__reference(dst, src, count, ditherMode);
#else
    mal_pcm_f32_to_s32__optimized(dst, src, count, ditherMode);
#endif
}


void mal_pcm_f32_to_f32(void* dst, const void* src, mal_uint64 count, mal_dither_mode ditherMode)
{
    (void)ditherMode;

    mal_copy_memory_64(dst, src, count * sizeof(float));
}


void mal_pcm_interleave_f32__reference(void* dst, const void** src, mal_uint64 frameCount, mal_uint32 channels)
{
    float* dst_f32 = (float*)dst;
    const float** src_f32 = (const float**)src;

    mal_uint64 iFrame;
    for (iFrame = 0; iFrame < frameCount; iFrame += 1) {
        mal_uint32 iChannel;
        for (iChannel = 0; iChannel < channels; iChannel += 1) {
            dst_f32[iFrame*channels + iChannel] = src_f32[iChannel][iFrame];
        }
    }
}

void mal_pcm_interleave_f32__optimized(void* dst, const void** src, mal_uint64 frameCount, mal_uint32 channels)
{
    mal_pcm_interleave_f32__reference(dst, src, frameCount, channels);
}

void mal_pcm_interleave_f32(void* dst, const void** src, mal_uint64 frameCount, mal_uint32 channels)
{
#ifdef MAL_USE_REFERENCE_CONVERSION_APIS
    mal_pcm_interleave_f32__reference(dst, src, frameCount, channels);
#else
    mal_pcm_interleave_f32__optimized(dst, src, frameCount, channels);
#endif
}


void mal_pcm_deinterleave_f32__reference(void** dst, const void* src, mal_uint64 frameCount, mal_uint32 channels)
{
    float** dst_f32 = (float**)dst;
    const float* src_f32 = (const float*)src;

    mal_uint64 iFrame;
    for (iFrame = 0; iFrame < frameCount; iFrame += 1) {
        mal_uint32 iChannel;
        for (iChannel = 0; iChannel < channels; iChannel += 1) {
            dst_f32[iChannel][iFrame] = src_f32[iFrame*channels + iChannel];
        }
    }
}

void mal_pcm_deinterleave_f32__optimized(void** dst, const void* src, mal_uint64 frameCount, mal_uint32 channels)
{
    mal_pcm_deinterleave_f32__reference(dst, src, frameCount, channels);
}

void mal_pcm_deinterleave_f32(void** dst, const void* src, mal_uint64 frameCount, mal_uint32 channels)
{
#ifdef MAL_USE_REFERENCE_CONVERSION_APIS
    mal_pcm_deinterleave_f32__reference(dst, src, frameCount, channels);
#else
    mal_pcm_deinterleave_f32__optimized(dst, src, frameCount, channels);
#endif
}


void mal_format_converter_init_callbacks__default(mal_format_converter* pConverter)
{
    mal_assert(pConverter != NULL);

    switch (pConverter->config.formatIn)
    {
        case mal_format_u8:
        {
            if (pConverter->config.formatOut == mal_format_u8) {
                pConverter->onConvertPCM = mal_pcm_u8_to_u8;
            } else if (pConverter->config.formatOut == mal_format_s16) {
                pConverter->onConvertPCM = mal_pcm_u8_to_s16;
            } else if (pConverter->config.formatOut == mal_format_s24) {
                pConverter->onConvertPCM = mal_pcm_u8_to_s24;
            } else if (pConverter->config.formatOut == mal_format_s32) {
                pConverter->onConvertPCM = mal_pcm_u8_to_s32;
            } else if (pConverter->config.formatOut == mal_format_f32) {
                pConverter->onConvertPCM = mal_pcm_u8_to_f32;
            }
        } break;

        case mal_format_s16:
        {
            if (pConverter->config.formatOut == mal_format_u8) {
                pConverter->onConvertPCM = mal_pcm_s16_to_u8;
            } else if (pConverter->config.formatOut == mal_format_s16) {
                pConverter->onConvertPCM = mal_pcm_s16_to_s16;
            } else if (pConverter->config.formatOut == mal_format_s24) {
                pConverter->onConvertPCM = mal_pcm_s16_to_s24;
            } else if (pConverter->config.formatOut == mal_format_s32) {
                pConverter->onConvertPCM = mal_pcm_s16_to_s32;
            } else if (pConverter->config.formatOut == mal_format_f32) {
                pConverter->onConvertPCM = mal_pcm_s16_to_f32;
            }
        } break;

        case mal_format_s24:
        {
            if (pConverter->config.formatOut == mal_format_u8) {
                pConverter->onConvertPCM = mal_pcm_s24_to_u8;
            } else if (pConverter->config.formatOut == mal_format_s16) {
                pConverter->onConvertPCM = mal_pcm_s24_to_s16;
            } else if (pConverter->config.formatOut == mal_format_s24) {
                pConverter->onConvertPCM = mal_pcm_s24_to_s24;
            } else if (pConverter->config.formatOut == mal_format_s32) {
                pConverter->onConvertPCM = mal_pcm_s24_to_s32;
            } else if (pConverter->config.formatOut == mal_format_f32) {
                pConverter->onConvertPCM = mal_pcm_s24_to_f32;
            }
        } break;

        case mal_format_s32:
        {
            if (pConverter->config.formatOut == mal_format_u8) {
                pConverter->onConvertPCM = mal_pcm_s32_to_u8;
            } else if (pConverter->config.formatOut == mal_format_s16) {
                pConverter->onConvertPCM = mal_pcm_s32_to_s16;
            } else if (pConverter->config.formatOut == mal_format_s24) {
                pConverter->onConvertPCM = mal_pcm_s32_to_s24;
            } else if (pConverter->config.formatOut == mal_format_s32) {
                pConverter->onConvertPCM = mal_pcm_s32_to_s32;
            } else if (pConverter->config.formatOut == mal_format_f32) {
                pConverter->onConvertPCM = mal_pcm_s32_to_f32;
            }
        } break;

        case mal_format_f32:
        default:
        {
            if (pConverter->config.formatOut == mal_format_u8) {
                pConverter->onConvertPCM = mal_pcm_f32_to_u8;
            } else if (pConverter->config.formatOut == mal_format_s16) {
                pConverter->onConvertPCM = mal_pcm_f32_to_s16;
            } else if (pConverter->config.formatOut == mal_format_s24) {
                pConverter->onConvertPCM = mal_pcm_f32_to_s24;
            } else if (pConverter->config.formatOut == mal_format_s32) {
                pConverter->onConvertPCM = mal_pcm_f32_to_s32;
            } else if (pConverter->config.formatOut == mal_format_f32) {
                pConverter->onConvertPCM = mal_pcm_f32_to_f32;
            }
        } break;
    }
}

#if defined(MAL_SUPPORT_SSE2)
void mal_format_converter_init_callbacks__sse2(mal_format_converter* pConverter)
{
    mal_assert(pConverter != NULL);

    switch (pConverter->config.formatIn)
    {
        case mal_format_u8:
        {
            if (pConverter->config.formatOut == mal_format_u8) {
                pConverter->onConvertPCM = mal_pcm_u8_to_u8;
            } else if (pConverter->config.formatOut == mal_format_s16) {
                pConverter->onConvertPCM = mal_pcm_u8_to_s16__sse2;
            } else if (pConverter->config.formatOut == mal_format_s24) {
                pConverter->onConvertPCM = mal_pcm_u8_to_s24__sse2;
            } else if (pConverter->config.formatOut == mal_format_s32) {
                pConverter->onConvertPCM = mal_pcm_u8_to_s32__sse2;
            } else if (pConverter->config.formatOut == mal_format_f32) {
                pConverter->onConvertPCM = mal_pcm_u8_to_f32__sse2;
            }
        } break;

        case mal_format_s16:
        {
            if (pConverter->config.formatOut == mal_format_u8) {
                pConverter->onConvertPCM = mal_pcm_s16_to_u8__sse2;
            } else if (pConverter->config.formatOut == mal_format_s16) {
                pConverter->onConvertPCM = mal_pcm_s16_to_s16;
            } else if (pConverter->config.formatOut == mal_format_s24) {
                pConverter->onConvertPCM = mal_pcm_s16_to_s24__sse2;
            } else if (pConverter->config.formatOut == mal_format_s32) {
                pConverter->onConvertPCM = mal_pcm_s16_to_s32__sse2;
            } else if (pConverter->config.formatOut == mal_format_f32) {
                pConverter->onConvertPCM = mal_pcm_s16_to_f32__sse2;
            }
        } break;

        case mal_format_s24:
        {
            if (pConverter->config.formatOut == mal_format_u8) {
                pConverter->onConvertPCM = mal_pcm_s24_to_u8__sse2;
            } else if (pConverter->config.formatOut == mal_format_s16) {
                pConverter->onConvertPCM = mal_pcm_s24_to_s16__sse2;
            } else if (pConverter->config.formatOut == mal_format_s24) {
                pConverter->onConvertPCM = mal_pcm_s24_to_s24;
            } else if (pConverter->config.formatOut == mal_format_s32) {
                pConverter->onConvertPCM = mal_pcm_s24_to_s32__sse2;
            } else if (pConverter->config.formatOut == mal_format_f32) {
                pConverter->onConvertPCM = mal_pcm_s24_to_f32__sse2;
            }
        } break;

        case mal_format_s32:
        {
            if (pConverter->config.formatOut == mal_format_u8) {
                pConverter->onConvertPCM = mal_pcm_s32_to_u8__sse2;
            } else if (pConverter->config.formatOut == mal_format_s16) {
                pConverter->onConvertPCM = mal_pcm_s32_to_s16__sse2;
            } else if (pConverter->config.formatOut == mal_format_s24) {
                pConverter->onConvertPCM = mal_pcm_s32_to_s24__sse2;
            } else if (pConverter->config.formatOut == mal_format_s32) {
                pConverter->onConvertPCM = mal_pcm_s32_to_s32;
            } else if (pConverter->config.formatOut == mal_format_f32) {
                pConverter->onConvertPCM = mal_pcm_s32_to_f32__sse2;
            }
        } break;

        case mal_format_f32:
        default:
        {
            if (pConverter->config.formatOut == mal_format_u8) {
                pConverter->onConvertPCM = mal_pcm_f32_to_u8__sse2;
            } else if (pConverter->config.formatOut == mal_format_s16) {
                pConverter->onConvertPCM = mal_pcm_f32_to_s16__sse2;
            } else if (pConverter->config.formatOut == mal_format_s24) {
                pConverter->onConvertPCM = mal_pcm_f32_to_s24__sse2;
            } else if (pConverter->config.formatOut == mal_format_s32) {
                pConverter->onConvertPCM = mal_pcm_f32_to_s32__sse2;
            } else if (pConverter->config.formatOut == mal_format_f32) {
                pConverter->onConvertPCM = mal_pcm_f32_to_f32;
            }
        } break;
    }
}
#endif

#if defined(MAL_SUPPORT_AVX2)
void mal_format_converter_init_callbacks__avx2(mal_format_converter* pConverter)
{
    mal_assert(pConverter != NULL);

    switch (pConverter->config.formatIn)
    {
        case mal_format_u8:
        {
            if (pConverter->config.formatOut == mal_format_u8) {
                pConverter->onConvertPCM = mal_pcm_u8_to_u8;
            } else if (pConverter->config.formatOut == mal_format_s16) {
                pConverter->onConvertPCM = mal_pcm_u8_to_s16__avx2;
            } else if (pConverter->config.formatOut == mal_format_s24) {
                pConverter->onConvertPCM = mal_pcm_u8_to_s24__avx2;
            } else if (pConverter->config.formatOut == mal_format_s32) {
                pConverter->onConvertPCM = mal_pcm_u8_to_s32__avx2;
            } else if (pConverter->config.formatOut == mal_format_f32) {
                pConverter->onConvertPCM = mal_pcm_u8_to_f32__avx2;
            }
        } break;

        case mal_format_s16:
        {
            if (pConverter->config.formatOut == mal_format_u8) {
                pConverter->onConvertPCM = mal_pcm_s16_to_u8__avx2;
            } else if (pConverter->config.formatOut == mal_format_s16) {
                pConverter->onConvertPCM = mal_pcm_s16_to_s16;
            } else if (pConverter->config.formatOut == mal_format_s24) {
                pConverter->onConvertPCM = mal_pcm_s16_to_s24__avx2;
            } else if (pConverter->config.formatOut == mal_format_s32) {
                pConverter->onConvertPCM = mal_pcm_s16_to_s32__avx2;
            } else if (pConverter->config.formatOut == mal_format_f32) {
                pConverter->onConvertPCM = mal_pcm_s16_to_f32__avx2;
            }
        } break;

        case mal_format_s24:
        {
            if (pConverter->config.formatOut == mal_format_u8) {
                pConverter->onConvertPCM = mal_pcm_s24_to_u8__avx2;
            } else if (pConverter->config.formatOut == mal_format_s16) {
                pConverter->onConvertPCM = mal_pcm_s24_to_s16__avx2;
            } else if (pConverter->config.formatOut == mal_format_s24) {
                pConverter->onConvertPCM = mal_pcm_s24_to_s24;
            } else if (pConverter->config.formatOut == mal_format_s32) {
                pConverter->onConvertPCM = mal_pcm_s24_to_s32__avx2;
            } else if (pConverter->config.formatOut == mal_format_f32) {
                pConverter->onConvertPCM = mal_pcm_s24_to_f32__avx2;
            }
        } break;

        case mal_format_s32:
        {
            if (pConverter->config.formatOut == mal_format_u8) {
                pConverter->onConvertPCM = mal_pcm_s32_to_u8__avx2;
            } else if (pConverter->config.formatOut == mal_format_s16) {
                pConverter->onConvertPCM = mal_pcm_s32_to_s16__avx2;
            } else if (pConverter->config.formatOut == mal_format_s24) {
                pConverter->onConvertPCM = mal_pcm_s32_to_s24__avx2;
            } else if (pConverter->config.formatOut == mal_format_s32) {
                pConverter->onConvertPCM = mal_pcm_s32_to_s32;
            } else if (pConverter->config.formatOut == mal_format_f32) {
                pConverter->onConvertPCM = mal_pcm_s32_to_f32__avx2;
            }
        } break;

        case mal_format_f32:
        default:
        {
            if (pConverter->config.formatOut == mal_format_u8) {
                pConverter->onConvertPCM = mal_pcm_f32_to_u8__avx2;
            } else if (pConverter->config.formatOut == mal_format_s16) {
                pConverter->onConvertPCM = mal_pcm_f32_to_s16__avx2;
            } else if (pConverter->config.formatOut == mal_format_s24) {
                pConverter->onConvertPCM = mal_pcm_f32_to_s24__avx2;
            } else if (pConverter->config.formatOut == mal_format_s32) {
                pConverter->onConvertPCM = mal_pcm_f32_to_s32__avx2;
            } else if (pConverter->config.formatOut == mal_format_f32) {
                pConverter->onConvertPCM = mal_pcm_f32_to_f32;
            }
        } break;
    }
}
#endif

#if defined(MAL_SUPPORT_AVX512)
void mal_format_converter_init_callbacks__avx512(mal_format_converter* pConverter)
{
    mal_assert(pConverter != NULL);

    switch (pConverter->config.formatIn)
    {
        case mal_format_u8:
        {
            if (pConverter->config.formatOut == mal_format_u8) {
                pConverter->onConvertPCM = mal_pcm_u8_to_u8;
            } else if (pConverter->config.formatOut == mal_format_s16) {
                pConverter->onConvertPCM = mal_pcm_u8_to_s16__avx512;
            } else if (pConverter->config.formatOut == mal_format_s24) {
                pConverter->onConvertPCM = mal_pcm_u8_to_s24__avx512;
            } else if (pConverter->config.formatOut == mal_format_s32) {
                pConverter->onConvertPCM = mal_pcm_u8_to_s32__avx512;
            } else if (pConverter->config.formatOut == mal_format_f32) {
                pConverter->onConvertPCM = mal_pcm_u8_to_f32__avx512;
            }
        } break;

        case mal_format_s16:
        {
            if (pConverter->config.formatOut == mal_format_u8) {
                pConverter->onConvertPCM = mal_pcm_s16_to_u8__avx512;
            } else if (pConverter->config.formatOut == mal_format_s16) {
                pConverter->onConvertPCM = mal_pcm_s16_to_s16;
            } else if (pConverter->config.formatOut == mal_format_s24) {
                pConverter->onConvertPCM = mal_pcm_s16_to_s24__avx512;
            } else if (pConverter->config.formatOut == mal_format_s32) {
                pConverter->onConvertPCM = mal_pcm_s16_to_s32__avx512;
            } else if (pConverter->config.formatOut == mal_format_f32) {
                pConverter->onConvertPCM = mal_pcm_s16_to_f32__avx512;
            }
        } break;

        case mal_format_s24:
        {
            if (pConverter->config.formatOut == mal_format_u8) {
                pConverter->onConvertPCM = mal_pcm_s24_to_u8__avx512;
            } else if (pConverter->config.formatOut == mal_format_s16) {
                pConverter->onConvertPCM = mal_pcm_s24_to_s16__avx512;
            } else if (pConverter->config.formatOut == mal_format_s24) {
                pConverter->onConvertPCM = mal_pcm_s24_to_s24;
            } else if (pConverter->config.formatOut == mal_format_s32) {
                pConverter->onConvertPCM = mal_pcm_s24_to_s32__avx512;
            } else if (pConverter->config.formatOut == mal_format_f32) {
                pConverter->onConvertPCM = mal_pcm_s24_to_f32__avx512;
            }
        } break;

        case mal_format_s32:
        {
            if (pConverter->config.formatOut == mal_format_u8) {
                pConverter->onConvertPCM = mal_pcm_s32_to_u8__avx512;
            } else if (pConverter->config.formatOut == mal_format_s16) {
                pConverter->onConvertPCM = mal_pcm_s32_to_s16__avx512;
            } else if (pConverter->config.formatOut == mal_format_s24) {
                pConverter->onConvertPCM = mal_pcm_s32_to_s24__avx512;
            } else if (pConverter->config.formatOut == mal_format_s32) {
                pConverter->onConvertPCM = mal_pcm_s32_to_s32;
            } else if (pConverter->config.formatOut == mal_format_f32) {
                pConverter->onConvertPCM = mal_pcm_s32_to_f32__avx512;
            }
        } break;

        case mal_format_f32:
        default:
        {
            if (pConverter->config.formatOut == mal_format_u8) {
                pConverter->onConvertPCM = mal_pcm_f32_to_u8__avx512;
            } else if (pConverter->config.formatOut == mal_format_s16) {
                pConverter->onConvertPCM = mal_pcm_f32_to_s16__avx512;
            } else if (pConverter->config.formatOut == mal_format_s24) {
                pConverter->onConvertPCM = mal_pcm_f32_to_s24__avx512;
            } else if (pConverter->config.formatOut == mal_format_s32) {
                pConverter->onConvertPCM = mal_pcm_f32_to_s32__avx512;
            } else if (pConverter->config.formatOut == mal_format_f32) {
                pConverter->onConvertPCM = mal_pcm_f32_to_f32;
            }
        } break;
    }
}
#endif

#if defined(MAL_SUPPORT_NEON)
void mal_format_converter_init_callbacks__neon(mal_format_converter* pConverter)
{
    mal_assert(pConverter != NULL);

    switch (pConverter->config.formatIn)
    {
        case mal_format_u8:
        {
            if (pConverter->config.formatOut == mal_format_u8) {
                pConverter->onConvertPCM = mal_pcm_u8_to_u8;
            } else if (pConverter->config.formatOut == mal_format_s16) {
                pConverter->onConvertPCM = mal_pcm_u8_to_s16__neon;
            } else if (pConverter->config.formatOut == mal_format_s24) {
                pConverter->onConvertPCM = mal_pcm_u8_to_s24__neon;
            } else if (pConverter->config.formatOut == mal_format_s32) {
                pConverter->onConvertPCM = mal_pcm_u8_to_s32__neon;
            } else if (pConverter->config.formatOut == mal_format_f32) {
                pConverter->onConvertPCM = mal_pcm_u8_to_f32__neon;
            }
        } break;

        case mal_format_s16:
        {
            if (pConverter->config.formatOut == mal_format_u8) {
                pConverter->onConvertPCM = mal_pcm_s16_to_u8__neon;
            } else if (pConverter->config.formatOut == mal_format_s16) {
                pConverter->onConvertPCM = mal_pcm_s16_to_s16;
            } else if (pConverter->config.formatOut == mal_format_s24) {
                pConverter->onConvertPCM = mal_pcm_s16_to_s24__neon;
            } else if (pConverter->config.formatOut == mal_format_s32) {
                pConverter->onConvertPCM = mal_pcm_s16_to_s32__neon;
            } else if (pConverter->config.formatOut == mal_format_f32) {
                pConverter->onConvertPCM = mal_pcm_s16_to_f32__neon;
            }
        } break;

        case mal_format_s24:
        {
            if (pConverter->config.formatOut == mal_format_u8) {
                pConverter->onConvertPCM = mal_pcm_s24_to_u8__neon;
            } else if (pConverter->config.formatOut == mal_format_s16) {
                pConverter->onConvertPCM = mal_pcm_s24_to_s16__neon;
            } else if (pConverter->config.formatOut == mal_format_s24) {
                pConverter->onConvertPCM = mal_pcm_s24_to_s24;
            } else if (pConverter->config.formatOut == mal_format_s32) {
                pConverter->onConvertPCM = mal_pcm_s24_to_s32__neon;
            } else if (pConverter->config.formatOut == mal_format_f32) {
                pConverter->onConvertPCM = mal_pcm_s24_to_f32__neon;
            }
        } break;

        case mal_format_s32:
        {
            if (pConverter->config.formatOut == mal_format_u8) {
                pConverter->onConvertPCM = mal_pcm_s32_to_u8__neon;
            } else if (pConverter->config.formatOut == mal_format_s16) {
                pConverter->onConvertPCM = mal_pcm_s32_to_s16__neon;
            } else if (pConverter->config.formatOut == mal_format_s24) {
                pConverter->onConvertPCM = mal_pcm_s32_to_s24__neon;
            } else if (pConverter->config.formatOut == mal_format_s32) {
                pConverter->onConvertPCM = mal_pcm_s32_to_s32;
            } else if (pConverter->config.formatOut == mal_format_f32) {
                pConverter->onConvertPCM = mal_pcm_s32_to_f32__neon;
            }
        } break;

        case mal_format_f32:
        default:
        {
            if (pConverter->config.formatOut == mal_format_u8) {
                pConverter->onConvertPCM = mal_pcm_f32_to_u8__neon;
            } else if (pConverter->config.formatOut == mal_format_s16) {
                pConverter->onConvertPCM = mal_pcm_f32_to_s16__neon;
            } else if (pConverter->config.formatOut == mal_format_s24) {
                pConverter->onConvertPCM = mal_pcm_f32_to_s24__neon;
            } else if (pConverter->config.formatOut == mal_format_s32) {
                pConverter->onConvertPCM = mal_pcm_f32_to_s32__neon;
            } else if (pConverter->config.formatOut == mal_format_f32) {
                pConverter->onConvertPCM = mal_pcm_f32_to_f32;
            }
        } break;
    }
}
#endif

mal_result mal_format_converter_init(const mal_format_converter_config* pConfig, mal_format_converter* pConverter)
{
    if (pConverter == NULL) {
        return MAL_INVALID_ARGS;
    }
    mal_zero_object(pConverter);

    if (pConfig == NULL) {
        return MAL_INVALID_ARGS;
    }

    pConverter->config = *pConfig;

    // SIMD
    pConverter->useSSE2   = mal_has_sse2()    && !pConfig->noSSE2;
    pConverter->useAVX2   = mal_has_avx2()    && !pConfig->noAVX2;
    pConverter->useAVX512 = mal_has_avx512f() && !pConfig->noAVX512;
    pConverter->useNEON   = mal_has_neon()    && !pConfig->noNEON;

#if defined(MAL_SUPPORT_AVX512)
    if (pConverter->useAVX512) {
        mal_format_converter_init_callbacks__avx512(pConverter);
    } else
#endif
#if defined(MAL_SUPPORT_AVX2)
    if (pConverter->useAVX2) {
        mal_format_converter_init_callbacks__avx2(pConverter);
    } else
#endif
#if defined(MAL_SUPPORT_SSE2)
    if (pConverter->useSSE2) {
        mal_format_converter_init_callbacks__sse2(pConverter);
    } else
#endif
#if defined(MAL_SUPPORT_NEON)
    if (pConverter->useNEON) {
        mal_format_converter_init_callbacks__neon(pConverter);
    } else
#endif
    {
        mal_format_converter_init_callbacks__default(pConverter);
    }

    switch (pConfig->formatOut)
    {
        case mal_format_u8:
        {
            pConverter->onInterleavePCM   = mal_pcm_interleave_u8;
            pConverter->onDeinterleavePCM = mal_pcm_deinterleave_u8;
        } break;
        case mal_format_s16:
        {
            pConverter->onInterleavePCM   = mal_pcm_interleave_s16;
            pConverter->onDeinterleavePCM = mal_pcm_deinterleave_s16;
        } break;
        case mal_format_s24:
        {
            pConverter->onInterleavePCM   = mal_pcm_interleave_s24;
            pConverter->onDeinterleavePCM = mal_pcm_deinterleave_s24;
        } break;
        case mal_format_s32:
        {
            pConverter->onInterleavePCM   = mal_pcm_interleave_s32;
            pConverter->onDeinterleavePCM = mal_pcm_deinterleave_s32;
        } break;
        case mal_format_f32:
        default:
        {
            pConverter->onInterleavePCM   = mal_pcm_interleave_f32;
            pConverter->onDeinterleavePCM = mal_pcm_deinterleave_f32;
        } break;
    }

    return MAL_SUCCESS;
}

mal_uint64 mal_format_converter_read(mal_format_converter* pConverter, mal_uint64 frameCount, void* pFramesOut, void* pUserData)
{
    if (pConverter == NULL || pFramesOut == NULL) {
        return 0;
    }

    mal_uint64 totalFramesRead = 0;
    mal_uint32 sampleSizeIn    = mal_get_bytes_per_sample(pConverter->config.formatIn);
    mal_uint32 sampleSizeOut   = mal_get_bytes_per_sample(pConverter->config.formatOut);
    //mal_uint32 frameSizeIn     = sampleSizeIn  * pConverter->config.channels;
    mal_uint32 frameSizeOut    = sampleSizeOut * pConverter->config.channels;
    mal_uint8* pNextFramesOut  = (mal_uint8*)pFramesOut;

    if (pConverter->config.onRead != NULL) {
        // Input data is interleaved.
        if (pConverter->config.formatIn == pConverter->config.formatOut) {
            // Pass through.
            while (totalFramesRead < frameCount) {
                mal_uint64 framesRemaining = (frameCount - totalFramesRead);
                mal_uint64 framesToReadRightNow = framesRemaining;
                if (framesToReadRightNow > 0xFFFFFFFF) {
                    framesToReadRightNow = 0xFFFFFFFF;
                }

                mal_uint32 framesJustRead = (mal_uint32)pConverter->config.onRead(pConverter, (mal_uint32)framesToReadRightNow, pNextFramesOut, pUserData);
                if (framesJustRead == 0) {
                    break;
                }

                totalFramesRead += framesJustRead;
                pNextFramesOut  += framesJustRead * frameSizeOut;

                if (framesJustRead < framesToReadRightNow) {
                    break;
                }
            }
        } else {
            // Conversion required.
            MAL_ALIGN(MAL_SIMD_ALIGNMENT) mal_uint8 temp[MAL_MAX_CHANNELS * MAL_MAX_PCM_SAMPLE_SIZE_IN_BYTES * 128];
            mal_assert(sizeof(temp) <= 0xFFFFFFFF);

            mal_uint32 maxFramesToReadAtATime = sizeof(temp) / sampleSizeIn / pConverter->config.channels;

            while (totalFramesRead < frameCount) {
                mal_uint64 framesRemaining = (frameCount - totalFramesRead);
                mal_uint64 framesToReadRightNow = framesRemaining;
                if (framesToReadRightNow > maxFramesToReadAtATime) {
                    framesToReadRightNow = maxFramesToReadAtATime;
                }

                mal_uint32 framesJustRead = (mal_uint32)pConverter->config.onRead(pConverter, (mal_uint32)framesToReadRightNow, temp, pUserData);
                if (framesJustRead == 0) {
                    break;
                }

                pConverter->onConvertPCM(pNextFramesOut, temp, framesJustRead*pConverter->config.channels, pConverter->config.ditherMode);

                totalFramesRead += framesJustRead;
                pNextFramesOut  += framesJustRead * frameSizeOut;

                if (framesJustRead < framesToReadRightNow) {
                    break;
                }
            }
        }
    } else {
        // Input data is deinterleaved. If a conversion is required we need to do an intermediary step.
        MAL_ALIGN(MAL_SIMD_ALIGNMENT) mal_uint8 tempSamplesOfOutFormat[MAL_MAX_CHANNELS * MAL_MAX_PCM_SAMPLE_SIZE_IN_BYTES * 128];
        mal_assert(sizeof(tempSamplesOfOutFormat) <= 0xFFFFFFFFF);

        void* ppTempSamplesOfOutFormat[MAL_MAX_CHANNELS];
        size_t splitBufferSizeOut;
        mal_split_buffer(tempSamplesOfOutFormat, sizeof(tempSamplesOfOutFormat), pConverter->config.channels, MAL_SIMD_ALIGNMENT, (void**)&ppTempSamplesOfOutFormat, &splitBufferSizeOut);

        mal_uint32 maxFramesToReadAtATime = (mal_uint32)(splitBufferSizeOut / sampleSizeIn);

        while (totalFramesRead < frameCount) {
            mal_uint64 framesRemaining = (frameCount - totalFramesRead);
            mal_uint64 framesToReadRightNow = framesRemaining;
            if (framesToReadRightNow > maxFramesToReadAtATime) {
                framesToReadRightNow = maxFramesToReadAtATime;
            }

            mal_uint32 framesJustRead = 0;

            if (pConverter->config.formatIn == pConverter->config.formatOut) {
                // Only interleaving.
                framesJustRead = (mal_uint32)pConverter->config.onReadDeinterleaved(pConverter, (mal_uint32)framesToReadRightNow, ppTempSamplesOfOutFormat, pUserData);
                if (framesJustRead == 0) {
                    break;
                }
            } else {
                // Interleaving + Conversion. Convert first, then interleave.
                MAL_ALIGN(MAL_SIMD_ALIGNMENT) mal_uint8 tempSamplesOfInFormat[MAL_MAX_CHANNELS * MAL_MAX_PCM_SAMPLE_SIZE_IN_BYTES * 128];

                void* ppTempSamplesOfInFormat[MAL_MAX_CHANNELS];
                size_t splitBufferSizeIn;
                mal_split_buffer(tempSamplesOfInFormat, sizeof(tempSamplesOfInFormat), pConverter->config.channels, MAL_SIMD_ALIGNMENT, (void**)&ppTempSamplesOfInFormat, &splitBufferSizeIn);

                if (framesToReadRightNow > (splitBufferSizeIn / sampleSizeIn)) {
                    framesToReadRightNow = (splitBufferSizeIn / sampleSizeIn);
                }

                framesJustRead = (mal_uint32)pConverter->config.onReadDeinterleaved(pConverter, (mal_uint32)framesToReadRightNow, ppTempSamplesOfInFormat, pUserData);
                if (framesJustRead == 0) {
                    break;
                }

                for (mal_uint32 iChannel = 0; iChannel < pConverter->config.channels; iChannel += 1) {
                    pConverter->onConvertPCM(ppTempSamplesOfOutFormat[iChannel], ppTempSamplesOfInFormat[iChannel], framesJustRead, pConverter->config.ditherMode);
                }
            }

            pConverter->onInterleavePCM(pNextFramesOut, (const void**)ppTempSamplesOfOutFormat, framesJustRead, pConverter->config.channels);

            totalFramesRead += framesJustRead;
            pNextFramesOut  += framesJustRead * frameSizeOut;

            if (framesJustRead < framesToReadRightNow) {
                break;
            }
        }
    }

    return totalFramesRead;
}

mal_uint64 mal_format_converter_read_deinterleaved(mal_format_converter* pConverter, mal_uint64 frameCount, void** ppSamplesOut, void* pUserData)
{
    if (pConverter == NULL || ppSamplesOut == NULL) {
        return 0;
    }

    mal_uint64 totalFramesRead = 0;
    mal_uint32 sampleSizeIn = mal_get_bytes_per_sample(pConverter->config.formatIn);
    mal_uint32 sampleSizeOut = mal_get_bytes_per_sample(pConverter->config.formatOut);

    mal_uint8* ppNextSamplesOut[MAL_MAX_CHANNELS];
    mal_copy_memory(ppNextSamplesOut, ppSamplesOut, sizeof(void*) * pConverter->config.channels);

    if (pConverter->config.onRead != NULL) {
        // Input data is interleaved.
        MAL_ALIGN(MAL_SIMD_ALIGNMENT) mal_uint8 tempSamplesOfOutFormat[MAL_MAX_CHANNELS * MAL_MAX_PCM_SAMPLE_SIZE_IN_BYTES * 128];
        mal_assert(sizeof(tempSamplesOfOutFormat) <= 0xFFFFFFFF);

        mal_uint32 maxFramesToReadAtATime = sizeof(tempSamplesOfOutFormat) / sampleSizeIn / pConverter->config.channels;

        while (totalFramesRead < frameCount) {
            mal_uint64 framesRemaining = (frameCount - totalFramesRead);
            mal_uint64 framesToReadRightNow = framesRemaining;
            if (framesToReadRightNow > maxFramesToReadAtATime) {
                framesToReadRightNow = maxFramesToReadAtATime;
            }

            mal_uint32 framesJustRead = 0;

            if (pConverter->config.formatIn == pConverter->config.formatOut) {
                // Only de-interleaving.
                framesJustRead = (mal_uint32)pConverter->config.onRead(pConverter, (mal_uint32)framesToReadRightNow, tempSamplesOfOutFormat, pUserData);
                if (framesJustRead == 0) {
                    break;
                }
            } else {
                // De-interleaving + Conversion. Convert first, then de-interleave.
                MAL_ALIGN(MAL_SIMD_ALIGNMENT) mal_uint8 tempSamplesOfInFormat[sizeof(tempSamplesOfOutFormat)];

                framesJustRead = (mal_uint32)pConverter->config.onRead(pConverter, (mal_uint32)framesToReadRightNow, tempSamplesOfInFormat, pUserData);
                if (framesJustRead == 0) {
                    break;
                }

                pConverter->onConvertPCM(tempSamplesOfOutFormat, tempSamplesOfInFormat, framesJustRead * pConverter->config.channels, pConverter->config.ditherMode);
            }

            pConverter->onDeinterleavePCM((void**)ppNextSamplesOut, tempSamplesOfOutFormat, framesJustRead, pConverter->config.channels);

            totalFramesRead += framesJustRead;
            for (mal_uint32 iChannel = 0; iChannel < pConverter->config.channels; ++iChannel) {
                ppNextSamplesOut[iChannel] += framesJustRead * sampleSizeOut;
            }

            if (framesJustRead < framesToReadRightNow) {
                break;
            }
        }
    } else {
        // Input data is deinterleaved.
        if (pConverter->config.formatIn == pConverter->config.formatOut) {
            // Pass through.
            while (totalFramesRead < frameCount) {
                mal_uint64 framesRemaining = (frameCount - totalFramesRead);
                mal_uint64 framesToReadRightNow = framesRemaining;
                if (framesToReadRightNow > 0xFFFFFFFF) {
                    framesToReadRightNow = 0xFFFFFFFF;
                }

                mal_uint32 framesJustRead = (mal_uint32)pConverter->config.onReadDeinterleaved(pConverter, (mal_uint32)framesToReadRightNow, (void**)ppNextSamplesOut, pUserData);
                if (framesJustRead == 0) {
                    break;
                }

                totalFramesRead += framesJustRead;
                for (mal_uint32 iChannel = 0; iChannel < pConverter->config.channels; ++iChannel) {
                    ppNextSamplesOut[iChannel] += framesJustRead * sampleSizeOut;
                }

                if (framesJustRead < framesToReadRightNow) {
                    break;
                }
            }
        } else {
            // Conversion required.
            MAL_ALIGN(MAL_SIMD_ALIGNMENT) mal_uint8 temp[MAL_MAX_CHANNELS][MAL_MAX_PCM_SAMPLE_SIZE_IN_BYTES * 128];
            mal_assert(sizeof(temp) <= 0xFFFFFFFF);

            void* ppTemp[MAL_MAX_CHANNELS];
            size_t splitBufferSize;
            mal_split_buffer(temp, sizeof(temp), pConverter->config.channels, MAL_SIMD_ALIGNMENT, (void**)&ppTemp, &splitBufferSize);

            mal_uint32 maxFramesToReadAtATime = (mal_uint32)(splitBufferSize / sampleSizeIn);

            while (totalFramesRead < frameCount) {
                mal_uint64 framesRemaining = (frameCount - totalFramesRead);
                mal_uint64 framesToReadRightNow = framesRemaining;
                if (framesToReadRightNow > maxFramesToReadAtATime) {
                    framesToReadRightNow = maxFramesToReadAtATime;
                }

                mal_uint32 framesJustRead = (mal_uint32)pConverter->config.onReadDeinterleaved(pConverter, (mal_uint32)framesToReadRightNow, ppTemp, pUserData);
                if (framesJustRead == 0) {
                    break;
                }

                for (mal_uint32 iChannel = 0; iChannel < pConverter->config.channels; iChannel += 1) {
                    pConverter->onConvertPCM(ppNextSamplesOut[iChannel], ppTemp[iChannel], framesJustRead, pConverter->config.ditherMode);
                    ppNextSamplesOut[iChannel] += framesJustRead * sampleSizeOut;
                }

                totalFramesRead += framesJustRead;

                if (framesJustRead < framesToReadRightNow) {
                    break;
                }
            }
        }
    }

    return totalFramesRead;
}


mal_format_converter_config mal_format_converter_config_init_new()
{
    mal_format_converter_config config;
    mal_zero_object(&config);

    return config;
}

mal_format_converter_config mal_format_converter_config_init(mal_format formatIn, mal_format formatOut, mal_uint32 channels, mal_format_converter_read_proc onRead, void* pUserData)
{
    mal_format_converter_config config = mal_format_converter_config_init_new();
    config.formatIn = formatIn;
    config.formatOut = formatOut;
    config.channels = channels;
    config.onRead = onRead;
    config.onReadDeinterleaved = NULL;
    config.pUserData = pUserData;

    return config;
}

mal_format_converter_config mal_format_converter_config_init_deinterleaved(mal_format formatIn, mal_format formatOut, mal_uint32 channels, mal_format_converter_read_deinterleaved_proc onReadDeinterleaved, void* pUserData)
{
    mal_format_converter_config config = mal_format_converter_config_init(formatIn, formatOut, channels, NULL, pUserData);
    config.onReadDeinterleaved = onReadDeinterleaved;

    return config;
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Channel Routing
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// -X = Left,   +X = Right
// -Y = Bottom, +Y = Top
// -Z = Front,  +Z = Back
typedef struct
{
    float x;
    float y;
    float z;
} mal_vec3;

static MAL_INLINE mal_vec3 mal_vec3f(float x, float y, float z)
{
    mal_vec3 r;
    r.x = x;
    r.y = y;
    r.z = z;

    return r;
}

static MAL_INLINE mal_vec3 mal_vec3_add(mal_vec3 a, mal_vec3 b)
{
    return mal_vec3f(
        a.x + b.x,
        a.y + b.y,
        a.z + b.z
    );
}

static MAL_INLINE mal_vec3 mal_vec3_sub(mal_vec3 a, mal_vec3 b)
{
    return mal_vec3f(
        a.x - b.x,
        a.y - b.y,
        a.z - b.z
    );
}

static MAL_INLINE mal_vec3 mal_vec3_mul(mal_vec3 a, mal_vec3 b)
{
    return mal_vec3f(
        a.x * b.x,
        a.y * b.y,
        a.z * b.z
    );
}

static MAL_INLINE mal_vec3 mal_vec3_div(mal_vec3 a, mal_vec3 b)
{
    return mal_vec3f(
        a.x / b.x,
        a.y / b.y,
        a.z / b.z
    );
}

static MAL_INLINE float mal_vec3_dot(mal_vec3 a, mal_vec3 b)
{
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

static MAL_INLINE float mal_vec3_length2(mal_vec3 a)
{
    return mal_vec3_dot(a, a);
}

static MAL_INLINE float mal_vec3_length(mal_vec3 a)
{
    return (float)sqrt(mal_vec3_length2(a));
}

static MAL_INLINE mal_vec3 mal_vec3_normalize(mal_vec3 a)
{
    float len = 1 / mal_vec3_length(a);

    mal_vec3 r;
    r.x = a.x * len;
    r.y = a.y * len;
    r.z = a.z * len;

    return r;
}

static MAL_INLINE float mal_vec3_distance(mal_vec3 a, mal_vec3 b)
{
    return mal_vec3_length(mal_vec3_sub(a, b));
}


#define MAL_PLANE_LEFT      0
#define MAL_PLANE_RIGHT     1
#define MAL_PLANE_FRONT     2
#define MAL_PLANE_BACK      3
#define MAL_PLANE_BOTTOM    4
#define MAL_PLANE_TOP       5

float g_malChannelPlaneRatios[MAL_CHANNEL_POSITION_COUNT][6] = {
    { 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f},  // MAL_CHANNEL_NONE
    { 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f},  // MAL_CHANNEL_MONO
    { 0.5f,  0.0f,  0.5f,  0.0f,  0.0f,  0.0f},  // MAL_CHANNEL_FRONT_LEFT
    { 0.0f,  0.5f,  0.5f,  0.0f,  0.0f,  0.0f},  // MAL_CHANNEL_FRONT_RIGHT
    { 0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f},  // MAL_CHANNEL_FRONT_CENTER
    { 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f},  // MAL_CHANNEL_LFE
    { 0.5f,  0.0f,  0.0f,  0.5f,  0.0f,  0.0f},  // MAL_CHANNEL_BACK_LEFT
    { 0.0f,  0.5f,  0.0f,  0.5f,  0.0f,  0.0f},  // MAL_CHANNEL_BACK_RIGHT
    { 0.25f, 0.0f,  0.75f, 0.0f,  0.0f,  0.0f},  // MAL_CHANNEL_FRONT_LEFT_CENTER
    { 0.0f,  0.25f, 0.75f, 0.0f,  0.0f,  0.0f},  // MAL_CHANNEL_FRONT_RIGHT_CENTER
    { 0.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f},  // MAL_CHANNEL_BACK_CENTER
    { 1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f},  // MAL_CHANNEL_SIDE_LEFT
    { 0.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f},  // MAL_CHANNEL_SIDE_RIGHT
    { 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  1.0f},  // MAL_CHANNEL_TOP_CENTER
    { 0.33f, 0.0f,  0.33f, 0.0f,  0.0f,  0.34f}, // MAL_CHANNEL_TOP_FRONT_LEFT
    { 0.0f,  0.0f,  0.5f,  0.0f,  0.0f,  0.5f},  // MAL_CHANNEL_TOP_FRONT_CENTER
    { 0.0f,  0.33f, 0.33f, 0.0f,  0.0f,  0.34f}, // MAL_CHANNEL_TOP_FRONT_RIGHT
    { 0.33f, 0.0f,  0.0f,  0.33f, 0.0f,  0.34f}, // MAL_CHANNEL_TOP_BACK_LEFT
    { 0.0f,  0.0f,  0.0f,  0.5f,  0.0f,  0.5f},  // MAL_CHANNEL_TOP_BACK_CENTER
    { 0.0f,  0.33f, 0.0f,  0.33f, 0.0f,  0.34f}, // MAL_CHANNEL_TOP_BACK_RIGHT
    { 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f},  // MAL_CHANNEL_AUX_0
    { 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f},  // MAL_CHANNEL_AUX_1
    { 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f},  // MAL_CHANNEL_AUX_2
    { 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f},  // MAL_CHANNEL_AUX_3
    { 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f},  // MAL_CHANNEL_AUX_4
    { 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f},  // MAL_CHANNEL_AUX_5
    { 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f},  // MAL_CHANNEL_AUX_6
    { 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f},  // MAL_CHANNEL_AUX_7
    { 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f},  // MAL_CHANNEL_AUX_8
    { 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f},  // MAL_CHANNEL_AUX_9
    { 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f},  // MAL_CHANNEL_AUX_10
    { 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f},  // MAL_CHANNEL_AUX_11
    { 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f},  // MAL_CHANNEL_AUX_12
    { 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f},  // MAL_CHANNEL_AUX_13
    { 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f},  // MAL_CHANNEL_AUX_14
    { 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f},  // MAL_CHANNEL_AUX_15
    { 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f},  // MAL_CHANNEL_AUX_16
    { 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f},  // MAL_CHANNEL_AUX_17
    { 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f},  // MAL_CHANNEL_AUX_18
    { 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f},  // MAL_CHANNEL_AUX_19
    { 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f},  // MAL_CHANNEL_AUX_20
    { 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f},  // MAL_CHANNEL_AUX_21
    { 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f},  // MAL_CHANNEL_AUX_22
    { 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f},  // MAL_CHANNEL_AUX_23
    { 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f},  // MAL_CHANNEL_AUX_24
    { 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f},  // MAL_CHANNEL_AUX_25
    { 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f},  // MAL_CHANNEL_AUX_26
    { 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f},  // MAL_CHANNEL_AUX_27
    { 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f},  // MAL_CHANNEL_AUX_28
    { 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f},  // MAL_CHANNEL_AUX_29
    { 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f},  // MAL_CHANNEL_AUX_30
    { 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f},  // MAL_CHANNEL_AUX_31
};

float mal_calculate_channel_position_planar_weight(mal_channel channelPositionA, mal_channel channelPositionB)
{
    // Imagine the following simplified example: You have a single input speaker which is the front/left speaker which you want to convert to
    // the following output configuration:
    //
    //  - front/left
    //  - side/left
    //  - back/left
    //
    // The front/left output is easy - it the same speaker position so it receives the full contribution of the front/left input. The amount
    // of contribution to apply to the side/left and back/left speakers, however, is a bit more complicated.
    //
    // Imagine the front/left speaker as emitting audio from two planes - the front plane and the left plane. You can think of the front/left
    // speaker emitting half of it's total volume from the front, and the other half from the left. Since part of it's volume is being emitted
    // from the left side, and the side/left and back/left channels also emit audio from the left plane, one would expect that they would
    // receive some amount of contribution from front/left speaker. The amount of contribution depends on how many planes are shared between
    // the two speakers. Note that in the examples below I've added a top/front/left speaker as an example just to show how the math works
    // across 3 spatial dimensions.
    //
    // The first thing to do is figure out how each speaker's volume is spread over each of plane:
    //  - front/left:     2 planes (front and left)      = 1/2 = half it's total volume on each plane
    //  - side/left:      1 plane (left only)            = 1/1 = entire volume from left plane
    //  - back/left:      2 planes (back and left)       = 1/2 = half it's total volume on each plane
    //  - top/front/left: 3 planes (top, front and left) = 1/3 = one third it's total volume on each plane
    //
    // The amount of volume each channel contributes to each of it's planes is what controls how much it is willing to given and take to other
    // channels on the same plane. The volume that is willing to the given by one channel is multiplied by the volume that is willing to be
    // taken by the other to produce the final contribution.

    // Contribution = Sum(Volume to Give * Volume to Take)
    float contribution = 
        g_malChannelPlaneRatios[channelPositionA][0] * g_malChannelPlaneRatios[channelPositionB][0] +
        g_malChannelPlaneRatios[channelPositionA][1] * g_malChannelPlaneRatios[channelPositionB][1] +
        g_malChannelPlaneRatios[channelPositionA][2] * g_malChannelPlaneRatios[channelPositionB][2] +
        g_malChannelPlaneRatios[channelPositionA][3] * g_malChannelPlaneRatios[channelPositionB][3] +
        g_malChannelPlaneRatios[channelPositionA][4] * g_malChannelPlaneRatios[channelPositionB][4] +
        g_malChannelPlaneRatios[channelPositionA][5] * g_malChannelPlaneRatios[channelPositionB][5];

    return contribution;
}

float mal_channel_router__calculate_input_channel_planar_weight(const mal_channel_router* pRouter, mal_channel channelPositionIn, mal_channel channelPositionOut)
{
    mal_assert(pRouter != NULL);
    (void)pRouter;

    return mal_calculate_channel_position_planar_weight(channelPositionIn, channelPositionOut);
}

mal_bool32 mal_channel_router__is_spatial_channel_position(const mal_channel_router* pRouter, mal_channel channelPosition)
{
    mal_assert(pRouter != NULL);
    (void)pRouter;

    if (channelPosition == MAL_CHANNEL_NONE || channelPosition == MAL_CHANNEL_MONO || channelPosition == MAL_CHANNEL_LFE) {
        return MAL_FALSE;
    }

    for (int i = 0; i < 6; ++i) {
        if (g_malChannelPlaneRatios[channelPosition][i] != 0) {
            return MAL_TRUE;
        }
    }

    return MAL_FALSE;
}

mal_result mal_channel_router_init(const mal_channel_router_config* pConfig, mal_channel_router* pRouter)
{
    if (pRouter == NULL) {
        return MAL_INVALID_ARGS;
    }

    mal_zero_object(pRouter);

    if (pConfig == NULL) {
        return MAL_INVALID_ARGS;
    }
    if (pConfig->onReadDeinterleaved == NULL) {
        return MAL_INVALID_ARGS;
    }

    if (!mal_channel_map_valid(pConfig->channelsIn, pConfig->channelMapIn)) {
        return MAL_INVALID_ARGS;    // Invalid input channel map.
    }
    if (!mal_channel_map_valid(pConfig->channelsOut, pConfig->channelMapOut)) {
        return MAL_INVALID_ARGS;    // Invalid output channel map.
    }

    pRouter->config = *pConfig;

    // SIMD
    pRouter->useSSE2   = mal_has_sse2()    && !pConfig->noSSE2;
    pRouter->useAVX2   = mal_has_avx2()    && !pConfig->noAVX2;
    pRouter->useAVX512 = mal_has_avx512f() && !pConfig->noAVX512;
    pRouter->useNEON   = mal_has_neon()    && !pConfig->noNEON;

    // If the input and output channels and channel maps are the same we should use a passthrough.
    if (pRouter->config.channelsIn == pRouter->config.channelsOut) {
        if (mal_channel_map_equal(pRouter->config.channelsIn, pRouter->config.channelMapIn, pRouter->config.channelMapOut)) {
            pRouter->isPassthrough = MAL_TRUE;
        }
        if (mal_channel_map_blank(pRouter->config.channelsIn, pRouter->config.channelMapIn) || mal_channel_map_blank(pRouter->config.channelsOut, pRouter->config.channelMapOut)) {
            pRouter->isPassthrough = MAL_TRUE;
        }
    }

    // Here is where we do a bit of pre-processing to know how each channel should be combined to make up the output. Rules:
    //
    // 1) If it's a passthrough, do nothing - it's just a simple memcpy().
    // 2) If the channel counts are the same and every channel position in the input map is present in the output map, use a
    //    simple shuffle. An example might be different 5.1 channel layouts.
    // 3) Otherwise channels are blended based on spatial locality.
    if (!pRouter->isPassthrough) {
        if (pRouter->config.channelsIn == pRouter->config.channelsOut) {
            mal_bool32 areAllChannelPositionsPresent = MAL_TRUE;
            for (mal_uint32 iChannelIn = 0; iChannelIn < pRouter->config.channelsIn; ++iChannelIn) {
                mal_bool32 isInputChannelPositionInOutput = MAL_FALSE;
                for (mal_uint32 iChannelOut = 0; iChannelOut < pRouter->config.channelsOut; ++iChannelOut) {
                    if (pRouter->config.channelMapIn[iChannelIn] == pRouter->config.channelMapOut[iChannelOut]) {
                        isInputChannelPositionInOutput = MAL_TRUE;
                        break;
                    }
                }

                if (!isInputChannelPositionInOutput) {
                    areAllChannelPositionsPresent = MAL_FALSE;
                    break;
                }
            }

            if (areAllChannelPositionsPresent) {
                pRouter->isSimpleShuffle = MAL_TRUE;

                // All the router will be doing is rearranging channels which means all we need to do is use a shuffling table which is just
                // a mapping between the index of the input channel to the index of the output channel.
                for (mal_uint32 iChannelIn = 0; iChannelIn < pRouter->config.channelsIn; ++iChannelIn) {
                    for (mal_uint32 iChannelOut = 0; iChannelOut < pRouter->config.channelsOut; ++iChannelOut) {
                        if (pRouter->config.channelMapIn[iChannelIn] == pRouter->config.channelMapOut[iChannelOut]) {
                            pRouter->shuffleTable[iChannelIn] = (mal_uint8)iChannelOut;
                            break;
                        }
                    }
                }
            }
        }
    }


    // Here is where weights are calculated. Note that we calculate the weights at all times, even when using a passthrough and simple
    // shuffling. We use different algorithms for calculating weights depending on our mixing mode.
    //
    // In simple mode we don't do any blending (except for converting between mono, which is done in a later step). Instead we just
    // map 1:1 matching channels. In this mode, if no channels in the input channel map correspond to anything in the output channel
    // map, nothing will be heard!

    // In all cases we need to make sure all channels that are present in both channel maps have a 1:1 mapping.
    for (mal_uint32 iChannelIn = 0; iChannelIn < pRouter->config.channelsIn; ++iChannelIn) {
        mal_channel channelPosIn = pRouter->config.channelMapIn[iChannelIn];

        for (mal_uint32 iChannelOut = 0; iChannelOut < pRouter->config.channelsOut; ++iChannelOut) {
            mal_channel channelPosOut = pRouter->config.channelMapOut[iChannelOut];

            if (channelPosIn == channelPosOut) {
                pRouter->weights[iChannelIn][iChannelOut] = 1;
            }
        }
    }

    // The mono channel is accumulated on all other channels, except LFE. Make sure in this loop we exclude output mono channels since
    // they were handled in the pass above.
    for (mal_uint32 iChannelIn = 0; iChannelIn < pRouter->config.channelsIn; ++iChannelIn) {
        mal_channel channelPosIn = pRouter->config.channelMapIn[iChannelIn];

        if (channelPosIn == MAL_CHANNEL_MONO) {
            for (mal_uint32 iChannelOut = 0; iChannelOut < pRouter->config.channelsOut; ++iChannelOut) {
                mal_channel channelPosOut = pRouter->config.channelMapOut[iChannelOut];

                if (channelPosOut != MAL_CHANNEL_NONE && channelPosOut != MAL_CHANNEL_MONO && channelPosOut != MAL_CHANNEL_LFE) {
                    pRouter->weights[iChannelIn][iChannelOut] = 1;
                }
            }
        }
    }

    // The output mono channel is the average of all non-none, non-mono and non-lfe input channels.
    {
        mal_uint32 len = 0;
        for (mal_uint32 iChannelIn = 0; iChannelIn < pRouter->config.channelsIn; ++iChannelIn) {
            mal_channel channelPosIn = pRouter->config.channelMapIn[iChannelIn];

            if (channelPosIn != MAL_CHANNEL_NONE && channelPosIn != MAL_CHANNEL_MONO && channelPosIn != MAL_CHANNEL_LFE) {
                len += 1;
            }
        }

        if (len > 0) {
            float monoWeight = 1.0f / len;

            for (mal_uint32 iChannelOut = 0; iChannelOut < pRouter->config.channelsOut; ++iChannelOut) {
                mal_channel channelPosOut = pRouter->config.channelMapOut[iChannelOut];

                if (channelPosOut == MAL_CHANNEL_MONO) {
                    for (mal_uint32 iChannelIn = 0; iChannelIn < pRouter->config.channelsIn; ++iChannelIn) {
                        mal_channel channelPosIn = pRouter->config.channelMapIn[iChannelIn];

                        if (channelPosIn != MAL_CHANNEL_NONE && channelPosIn != MAL_CHANNEL_MONO && channelPosIn != MAL_CHANNEL_LFE) {
                            pRouter->weights[iChannelIn][iChannelOut] += monoWeight;
                        }
                    }
                }
            }
        }
    }


    // Input and output channels that are not present on the other side need to be blended in based on spatial locality.
    if (pRouter->config.mixingMode != mal_channel_mix_mode_simple) {
        // Unmapped input channels.
        for (mal_uint32 iChannelIn = 0; iChannelIn < pRouter->config.channelsIn; ++iChannelIn) {
            mal_channel channelPosIn = pRouter->config.channelMapIn[iChannelIn];

            if (mal_channel_router__is_spatial_channel_position(pRouter, channelPosIn)) {
                if (!mal_channel_map_contains_channel_position(pRouter->config.channelsOut, pRouter->config.channelMapOut, channelPosIn)) {
                    for (mal_uint32 iChannelOut = 0; iChannelOut < pRouter->config.channelsOut; ++iChannelOut) {
                        mal_channel channelPosOut = pRouter->config.channelMapOut[iChannelOut];

                        if (mal_channel_router__is_spatial_channel_position(pRouter, channelPosOut)) {
                            float weight = 0;
                            if (pRouter->config.mixingMode == mal_channel_mix_mode_planar_blend) {
                                weight = mal_channel_router__calculate_input_channel_planar_weight(pRouter, channelPosIn, channelPosOut);
                            }

                            // Only apply the weight if we haven't already got some contribution from the respective channels.
                            if (pRouter->weights[iChannelIn][iChannelOut] == 0) {
                                pRouter->weights[iChannelIn][iChannelOut] = weight;
                            }
                        }
                    }
                }
            }
        }

        // Unmapped output channels.
        for (mal_uint32 iChannelOut = 0; iChannelOut < pRouter->config.channelsOut; ++iChannelOut) {
            mal_channel channelPosOut = pRouter->config.channelMapOut[iChannelOut];

            if (mal_channel_router__is_spatial_channel_position(pRouter, channelPosOut)) {
                if (!mal_channel_map_contains_channel_position(pRouter->config.channelsIn, pRouter->config.channelMapIn, channelPosOut)) {
                    for (mal_uint32 iChannelIn = 0; iChannelIn < pRouter->config.channelsIn; ++iChannelIn) {
                        mal_channel channelPosIn = pRouter->config.channelMapIn[iChannelIn];

                        if (mal_channel_router__is_spatial_channel_position(pRouter, channelPosIn)) {
                            float weight = 0;
                            if (pRouter->config.mixingMode == mal_channel_mix_mode_planar_blend) {
                                weight = mal_channel_router__calculate_input_channel_planar_weight(pRouter, channelPosIn, channelPosOut);
                            }

                            // Only apply the weight if we haven't already got some contribution from the respective channels.
                            if (pRouter->weights[iChannelIn][iChannelOut] == 0) {
                                pRouter->weights[iChannelIn][iChannelOut] = weight;
                            }
                        }
                    }
                }
            }
        }
    }

    return MAL_SUCCESS;
}

static MAL_INLINE mal_bool32 mal_channel_router__can_use_sse2(mal_channel_router* pRouter, const float* pSamplesOut, const float* pSamplesIn)
{
    return pRouter->useSSE2 && (((mal_uintptr)pSamplesOut & 15) == 0) && (((mal_uintptr)pSamplesIn & 15) == 0);
}

static MAL_INLINE mal_bool32 mal_channel_router__can_use_avx2(mal_channel_router* pRouter, const float* pSamplesOut, const float* pSamplesIn)
{
    return pRouter->useAVX2 && (((mal_uintptr)pSamplesOut & 31) == 0) && (((mal_uintptr)pSamplesIn & 31) == 0);
}

static MAL_INLINE mal_bool32 mal_channel_router__can_use_avx512(mal_channel_router* pRouter, const float* pSamplesOut, const float* pSamplesIn)
{
    return pRouter->useAVX512 && (((mal_uintptr)pSamplesOut & 63) == 0) && (((mal_uintptr)pSamplesIn & 63) == 0);
}

static MAL_INLINE mal_bool32 mal_channel_router__can_use_neon(mal_channel_router* pRouter, const float* pSamplesOut, const float* pSamplesIn)
{
    return pRouter->useNEON && (((mal_uintptr)pSamplesOut & 15) == 0) && (((mal_uintptr)pSamplesIn & 15) == 0);
}

void mal_channel_router__do_routing(mal_channel_router* pRouter, mal_uint64 frameCount, float** ppSamplesOut, const float** ppSamplesIn)
{
    mal_assert(pRouter != NULL);
    mal_assert(pRouter->isPassthrough == MAL_FALSE);

    if (pRouter->isSimpleShuffle) {
        // A shuffle is just a re-arrangement of channels and does not require any arithmetic.
        mal_assert(pRouter->config.channelsIn == pRouter->config.channelsOut);
        for (mal_uint32 iChannelIn = 0; iChannelIn < pRouter->config.channelsIn; ++iChannelIn) {
            mal_uint32 iChannelOut = pRouter->shuffleTable[iChannelIn];
            mal_copy_memory_64(ppSamplesOut[iChannelOut], ppSamplesIn[iChannelIn], frameCount * sizeof(float));
        }
    } else {
        // This is the more complicated case. Each of the output channels is accumulated with 0 or more input channels.

        // Clear.
        for (mal_uint32 iChannelOut = 0; iChannelOut < pRouter->config.channelsOut; ++iChannelOut) {
            mal_zero_memory_64(ppSamplesOut[iChannelOut], frameCount * sizeof(float));
        }

        // Accumulate.
        for (mal_uint32 iChannelIn = 0; iChannelIn < pRouter->config.channelsIn; ++iChannelIn) {
            for (mal_uint32 iChannelOut = 0; iChannelOut < pRouter->config.channelsOut; ++iChannelOut) {
                mal_uint64 iFrame = 0;
#if defined(MAL_SUPPORT_NEON)
                if (mal_channel_router__can_use_neon(pRouter, ppSamplesOut[iChannelOut], ppSamplesIn[iChannelIn])) {
                    float32x4_t weight = vmovq_n_f32(pRouter->weights[iChannelIn][iChannelOut]);

                    mal_uint64 frameCount4 = frameCount/4;
                    for (mal_uint64 iFrame4 = 0; iFrame4 < frameCount4; iFrame4 += 1) {
                        float32x4_t* pO = (float32x4_t*)ppSamplesOut[iChannelOut] + iFrame4;
                        float32x4_t* pI = (float32x4_t*)ppSamplesIn [iChannelIn ] + iFrame4;
                        *pO = vaddq_f32(*pO, vmulq_f32(*pI, weight));
                    }

                    iFrame += frameCount4*4;
                }
                else
#endif
#if defined(MAL_SUPPORT_AVX512)
                if (mal_channel_router__can_use_avx512(pRouter, ppSamplesOut[iChannelOut], ppSamplesIn[iChannelIn])) {
                    __m512 weight = _mm512_set1_ps(pRouter->weights[iChannelIn][iChannelOut]);

                    mal_uint64 frameCount16 = frameCount/16;
                    for (mal_uint64 iFrame16 = 0; iFrame16 < frameCount16; iFrame16 += 1) {
                        __m512* pO = (__m512*)ppSamplesOut[iChannelOut] + iFrame16;
                        __m512* pI = (__m512*)ppSamplesIn [iChannelIn ] + iFrame16;
                        *pO = _mm512_add_ps(*pO, _mm512_mul_ps(*pI, weight));
                    }

                    iFrame += frameCount16*16;
                }
                else
#endif
#if defined(MAL_SUPPORT_AVX2)
                if (mal_channel_router__can_use_avx2(pRouter, ppSamplesOut[iChannelOut], ppSamplesIn[iChannelIn])) {
                    __m256 weight = _mm256_set1_ps(pRouter->weights[iChannelIn][iChannelOut]);

                    mal_uint64 frameCount8 = frameCount/8;
                    for (mal_uint64 iFrame8 = 0; iFrame8 < frameCount8; iFrame8 += 1) {
                        __m256* pO = (__m256*)ppSamplesOut[iChannelOut] + iFrame8;
                        __m256* pI = (__m256*)ppSamplesIn [iChannelIn ] + iFrame8;
                        *pO = _mm256_add_ps(*pO, _mm256_mul_ps(*pI, weight));
                    }

                    iFrame += frameCount8*8;
                }
                else
#endif
#if defined(MAL_SUPPORT_SSE2)
                if (mal_channel_router__can_use_sse2(pRouter, ppSamplesOut[iChannelOut], ppSamplesIn[iChannelIn])) {
                    __m128 weight = _mm_set1_ps(pRouter->weights[iChannelIn][iChannelOut]);

                    mal_uint64 frameCount4 = frameCount/4;
                    for (mal_uint64 iFrame4 = 0; iFrame4 < frameCount4; iFrame4 += 1) {
                        __m128* pO = (__m128*)ppSamplesOut[iChannelOut] + iFrame4;
                        __m128* pI = (__m128*)ppSamplesIn [iChannelIn ] + iFrame4;
                        *pO = _mm_add_ps(*pO, _mm_mul_ps(*pI, weight));
                    }

                    iFrame += frameCount4*4;
                } else 
#endif
                {   // Reference.
                    float weight0 = pRouter->weights[iChannelIn][iChannelOut];
                    float weight1 = pRouter->weights[iChannelIn][iChannelOut];
                    float weight2 = pRouter->weights[iChannelIn][iChannelOut];
                    float weight3 = pRouter->weights[iChannelIn][iChannelOut];

                    mal_uint64 frameCount4 = frameCount/4;
                    for (mal_uint64 iFrame4 = 0; iFrame4 < frameCount4; iFrame4 += 1) {
                        ppSamplesOut[iChannelOut][iFrame+0] += ppSamplesIn[iChannelIn][iFrame+0] * weight0;
                        ppSamplesOut[iChannelOut][iFrame+1] += ppSamplesIn[iChannelIn][iFrame+1] * weight1;
                        ppSamplesOut[iChannelOut][iFrame+2] += ppSamplesIn[iChannelIn][iFrame+2] * weight2;
                        ppSamplesOut[iChannelOut][iFrame+3] += ppSamplesIn[iChannelIn][iFrame+3] * weight3;
                        iFrame += 4;
                    }
                }

                // Leftover.
                for (; iFrame < frameCount; ++iFrame) {
                    ppSamplesOut[iChannelOut][iFrame] += ppSamplesIn[iChannelIn][iFrame] * pRouter->weights[iChannelIn][iChannelOut];
                }
            }
        }
    }
}

mal_uint64 mal_channel_router_read_deinterleaved(mal_channel_router* pRouter, mal_uint64 frameCount, void** ppSamplesOut, void* pUserData)
{
    if (pRouter == NULL || ppSamplesOut == NULL) {
        return 0;
    }

    // Fast path for a passthrough.
    if (pRouter->isPassthrough) {
        if (frameCount <= 0xFFFFFFFF) {
            return (mal_uint32)pRouter->config.onReadDeinterleaved(pRouter, (mal_uint32)frameCount, ppSamplesOut, pUserData);
        } else {
            float* ppNextSamplesOut[MAL_MAX_CHANNELS];
            mal_copy_memory(ppNextSamplesOut, ppSamplesOut, sizeof(float*) * pRouter->config.channelsOut);

            mal_uint64 totalFramesRead = 0;
            while (totalFramesRead < frameCount) {
                mal_uint64 framesRemaining = (frameCount - totalFramesRead);
                mal_uint64 framesToReadRightNow = framesRemaining;
                if (framesToReadRightNow > 0xFFFFFFFF) {
                    framesToReadRightNow = 0xFFFFFFFF;
                }

                mal_uint32 framesJustRead = (mal_uint32)pRouter->config.onReadDeinterleaved(pRouter, (mal_uint32)framesToReadRightNow, (void**)ppNextSamplesOut, pUserData);
                if (framesJustRead == 0) {
                    break;
                }

                totalFramesRead += framesJustRead;
                for (mal_uint32 iChannel = 0; iChannel < pRouter->config.channelsOut; ++iChannel) {
                    ppNextSamplesOut[iChannel] += framesJustRead;
                }

                if (framesJustRead < framesToReadRightNow) {
                    break;
                }
            }
        }
    }

    // Slower path for a non-passthrough.
    float* ppNextSamplesOut[MAL_MAX_CHANNELS];
    mal_copy_memory(ppNextSamplesOut, ppSamplesOut, sizeof(float*) * pRouter->config.channelsOut);

    MAL_ALIGN(MAL_SIMD_ALIGNMENT) float temp[MAL_MAX_CHANNELS * 256];
    mal_assert(sizeof(temp) <= 0xFFFFFFFF);

    float* ppTemp[MAL_MAX_CHANNELS];
    size_t maxBytesToReadPerFrameEachIteration;
    mal_split_buffer(temp, sizeof(temp), pRouter->config.channelsIn, MAL_SIMD_ALIGNMENT, (void**)&ppTemp, &maxBytesToReadPerFrameEachIteration);

    size_t maxFramesToReadEachIteration = maxBytesToReadPerFrameEachIteration/sizeof(float);

    mal_uint64 totalFramesRead = 0;
    while (totalFramesRead < frameCount) {
        mal_uint64 framesRemaining = (frameCount - totalFramesRead);
        mal_uint64 framesToReadRightNow = framesRemaining;
        if (framesToReadRightNow > maxFramesToReadEachIteration) {
            framesToReadRightNow = maxFramesToReadEachIteration;
        }

        mal_uint32 framesJustRead = pRouter->config.onReadDeinterleaved(pRouter, (mal_uint32)framesToReadRightNow, (void**)ppTemp, pUserData);
        if (framesJustRead == 0) {
            break;
        }

        mal_channel_router__do_routing(pRouter, framesJustRead, (float**)ppNextSamplesOut, (const float**)ppTemp);  // <-- Real work is done here.

        totalFramesRead += framesJustRead;
        if (totalFramesRead < frameCount) {
            for (mal_uint32 iChannel = 0; iChannel < pRouter->config.channelsIn; iChannel += 1) {
                ppNextSamplesOut[iChannel] += framesJustRead;
            }
        }

        if (framesJustRead < framesToReadRightNow) {
            break;
        }
    }

    return totalFramesRead;
}

mal_channel_router_config mal_channel_router_config_init(mal_uint32 channelsIn, const mal_channel channelMapIn[MAL_MAX_CHANNELS], mal_uint32 channelsOut, const mal_channel channelMapOut[MAL_MAX_CHANNELS], mal_channel_mix_mode mixingMode, mal_channel_router_read_deinterleaved_proc onRead, void* pUserData)
{
    mal_channel_router_config config;
    mal_zero_object(&config);

    config.channelsIn = channelsIn;
    for (mal_uint32 iChannel = 0; iChannel < channelsIn; ++iChannel) {
        config.channelMapIn[iChannel] = channelMapIn[iChannel];
    }

    config.channelsOut = channelsOut;
    for (mal_uint32 iChannel = 0; iChannel < channelsOut; ++iChannel) {
        config.channelMapOut[iChannel] = channelMapOut[iChannel];
    }

    config.mixingMode = mixingMode;
    config.onReadDeinterleaved = onRead;
    config.pUserData = pUserData;

    return config;
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// SRC
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define mal_floorf(x) ((float)floor((double)(x)))
#define mal_sinf(x)   ((float)sin((double)(x)))
#define mal_cosf(x)   ((float)cos((double)(x)))

static MAL_INLINE double mal_sinc(double x)
{
    if (x != 0) {
        return sin(MAL_PI_D*x) / (MAL_PI_D*x);
    } else {
        return 1;
    }
}

#define mal_sincf(x) ((float)mal_sinc((double)(x)))

mal_uint64 mal_calculate_frame_count_after_src(mal_uint32 sampleRateOut, mal_uint32 sampleRateIn, mal_uint64 frameCountIn)
{
    double srcRatio = (double)sampleRateOut / sampleRateIn;
    double frameCountOutF = frameCountIn * srcRatio;

    mal_uint64 frameCountOut = (mal_uint64)frameCountOutF;

    // If the output frame count is fractional, make sure we add an extra frame to ensure there's enough room for that last sample.
    if ((frameCountOutF - frameCountOut) > 0.0) {
        frameCountOut += 1;
    }

    return frameCountOut;
}


mal_uint64 mal_src_read_deinterleaved__passthrough(mal_src* pSRC, mal_uint64 frameCount, void** ppSamplesOut, void* pUserData);
mal_uint64 mal_src_read_deinterleaved__linear(mal_src* pSRC, mal_uint64 frameCount, void** ppSamplesOut, void* pUserData);
mal_uint64 mal_src_read_deinterleaved__sinc(mal_src* pSRC, mal_uint64 frameCount, void** ppSamplesOut, void* pUserData);

void mal_src__build_sinc_table__sinc(mal_src* pSRC)
{
    mal_assert(pSRC != NULL);

    pSRC->sinc.table[0] = 1.0f;
    for (mal_uint32 i = 1; i < mal_countof(pSRC->sinc.table); i += 1) {
        double x = i*MAL_PI_D / MAL_SRC_SINC_LOOKUP_TABLE_RESOLUTION;
        pSRC->sinc.table[i] = (float)(sin(x)/x);
    }
}

void mal_src__build_sinc_table__rectangular(mal_src* pSRC)
{
    // This is the same as the base sinc table.
    mal_src__build_sinc_table__sinc(pSRC);
}

void mal_src__build_sinc_table__hann(mal_src* pSRC)
{
    mal_src__build_sinc_table__sinc(pSRC);

    for (mal_uint32 i = 0; i < mal_countof(pSRC->sinc.table); i += 1) {
        double x = pSRC->sinc.table[i];
        double N = MAL_SRC_SINC_MAX_WINDOW_WIDTH*2;
        double n = ((double)(i) / MAL_SRC_SINC_LOOKUP_TABLE_RESOLUTION) + MAL_SRC_SINC_MAX_WINDOW_WIDTH;
        double w = 0.5 * (1 - cos((2*MAL_PI_D*n) / (N)));

        pSRC->sinc.table[i] = (float)(x * w);
    }
}

mal_result mal_src_init(const mal_src_config* pConfig, mal_src* pSRC)
{
    if (pSRC == NULL) {
        return MAL_INVALID_ARGS;
    }

    mal_zero_object(pSRC);

    if (pConfig == NULL || pConfig->onReadDeinterleaved == NULL) {
        return MAL_INVALID_ARGS;
    }
    if (pConfig->channels == 0 || pConfig->channels > MAL_MAX_CHANNELS) {
        return MAL_INVALID_ARGS;
    }

    pSRC->config = *pConfig;

    // SIMD
    pSRC->useSSE2   = mal_has_sse2()    && !pConfig->noSSE2;
    pSRC->useAVX2   = mal_has_avx2()    && !pConfig->noAVX2;
    pSRC->useAVX512 = mal_has_avx512f() && !pConfig->noAVX512;
    pSRC->useNEON   = mal_has_neon()    && !pConfig->noNEON;

    if (pSRC->config.algorithm == mal_src_algorithm_sinc) {
        // Make sure the window width within bounds.
        if (pSRC->config.sinc.windowWidth == 0) {
            pSRC->config.sinc.windowWidth = MAL_SRC_SINC_DEFAULT_WINDOW_WIDTH;
        }
        if (pSRC->config.sinc.windowWidth < MAL_SRC_SINC_MIN_WINDOW_WIDTH) {
            pSRC->config.sinc.windowWidth = MAL_SRC_SINC_MIN_WINDOW_WIDTH;
        }
        if (pSRC->config.sinc.windowWidth > MAL_SRC_SINC_MAX_WINDOW_WIDTH) {
            pSRC->config.sinc.windowWidth = MAL_SRC_SINC_MAX_WINDOW_WIDTH;
        }

        // Set up the lookup table.
        switch (pSRC->config.sinc.windowFunction) {
            case mal_src_sinc_window_function_hann:        mal_src__build_sinc_table__hann(pSRC);        break;
            case mal_src_sinc_window_function_rectangular: mal_src__build_sinc_table__rectangular(pSRC); break;
            default: return MAL_INVALID_ARGS;   // <-- Hitting this means the window function is unknown to mini_al.
        }
    }

    return MAL_SUCCESS;
}

mal_result mal_src_set_input_sample_rate(mal_src* pSRC, mal_uint32 sampleRateIn)
{
    if (pSRC == NULL) {
        return MAL_INVALID_ARGS;
    }

    // Must have a sample rate of > 0.
    if (sampleRateIn == 0) {
        return MAL_INVALID_ARGS;
    }

    mal_atomic_exchange_32(&pSRC->config.sampleRateIn, sampleRateIn);
    return MAL_SUCCESS;
}

mal_result mal_src_set_output_sample_rate(mal_src* pSRC, mal_uint32 sampleRateOut)
{
    if (pSRC == NULL) {
        return MAL_INVALID_ARGS;
    }

    // Must have a sample rate of > 0.
    if (sampleRateOut == 0) {
        return MAL_INVALID_ARGS;
    }

    mal_atomic_exchange_32(&pSRC->config.sampleRateOut, sampleRateOut);
    return MAL_SUCCESS;
}

mal_result mal_src_set_sample_rate(mal_src* pSRC, mal_uint32 sampleRateIn, mal_uint32 sampleRateOut)
{
    if (pSRC == NULL) {
        return MAL_INVALID_ARGS;
    }

    // Must have a sample rate of > 0.
    if (sampleRateIn == 0 || sampleRateOut == 0) {
        return MAL_INVALID_ARGS;
    }

    mal_atomic_exchange_32(&pSRC->config.sampleRateIn, sampleRateIn);
    mal_atomic_exchange_32(&pSRC->config.sampleRateOut, sampleRateOut);

    return MAL_SUCCESS;
}

mal_uint64 mal_src_read_deinterleaved(mal_src* pSRC, mal_uint64 frameCount, void** ppSamplesOut, void* pUserData)
{
    if (pSRC == NULL || frameCount == 0 || ppSamplesOut == NULL) {
        return 0;
    }

    mal_src_algorithm algorithm = pSRC->config.algorithm;

    // Can use a function pointer for this.
    switch (algorithm) {
        case mal_src_algorithm_none:   return mal_src_read_deinterleaved__passthrough(pSRC, frameCount, ppSamplesOut, pUserData);
        case mal_src_algorithm_linear: return mal_src_read_deinterleaved__linear(     pSRC, frameCount, ppSamplesOut, pUserData);
        case mal_src_algorithm_sinc:   return mal_src_read_deinterleaved__sinc(       pSRC, frameCount, ppSamplesOut, pUserData);
        default: break;
    }

    // Should never get here.
    return 0;
}

mal_uint64 mal_src_read_deinterleaved__passthrough(mal_src* pSRC, mal_uint64 frameCount, void** ppSamplesOut, void* pUserData)
{
    if (frameCount <= 0xFFFFFFFF) {
        return pSRC->config.onReadDeinterleaved(pSRC, (mal_uint32)frameCount, ppSamplesOut, pUserData);
    } else {
        float* ppNextSamplesOut[MAL_MAX_CHANNELS];
        for (mal_uint32 iChannel = 0; iChannel < pSRC->config.channels; ++iChannel) {
            ppNextSamplesOut[iChannel] = (float*)ppSamplesOut[iChannel];
        }

        mal_uint64 totalFramesRead = 0;
        while (totalFramesRead < frameCount) {
            mal_uint64 framesRemaining = frameCount - totalFramesRead;
            mal_uint64 framesToReadRightNow = framesRemaining;
            if (framesToReadRightNow > 0xFFFFFFFF) {
                framesToReadRightNow = 0xFFFFFFFF;
            }

            mal_uint32 framesJustRead = (mal_uint32)pSRC->config.onReadDeinterleaved(pSRC, (mal_uint32)framesToReadRightNow, (void**)ppNextSamplesOut, pUserData);
            if (framesJustRead == 0) {
                break;
            }

            totalFramesRead += framesJustRead;
            for (mal_uint32 iChannel = 0; iChannel < pSRC->config.channels; ++iChannel) {
                ppNextSamplesOut[iChannel] += framesJustRead;
            }

            if (framesJustRead < framesToReadRightNow) {
                break;
            }
        }

        return totalFramesRead;
    }
}

mal_uint64 mal_src_read_deinterleaved__linear(mal_src* pSRC, mal_uint64 frameCount, void** ppSamplesOut, void* pUserData)
{
    mal_assert(pSRC != NULL);
    mal_assert(frameCount > 0);
    mal_assert(ppSamplesOut != NULL);

    float* ppNextSamplesOut[MAL_MAX_CHANNELS];
    mal_copy_memory(ppNextSamplesOut, ppSamplesOut, sizeof(void*) * pSRC->config.channels);


    float factor = (float)pSRC->config.sampleRateIn / pSRC->config.sampleRateOut;

    mal_uint32 maxFrameCountPerChunkIn = mal_countof(pSRC->linear.input[0]);

    mal_uint64 totalFramesRead = 0;
    while (totalFramesRead < frameCount) {
        mal_uint64 framesRemaining = frameCount - totalFramesRead;
        mal_uint64 framesToRead = framesRemaining;
        if (framesToRead > 16384) {
            framesToRead = 16384;    // <-- Keep this small because we're using 32-bit floats for calculating sample positions and I don't want to run out of precision with huge sample counts.
        }


        // Read Input Data
        // ===============
        float tBeg = pSRC->linear.timeIn;
        float tEnd = tBeg + (framesToRead*factor);

        mal_uint32 framesToReadFromClient = (mal_uint32)(tEnd) + 1 + 1;   // +1 to make tEnd 1-based and +1 because we always need to an extra sample for interpolation.
        if (framesToReadFromClient >= maxFrameCountPerChunkIn) {
            framesToReadFromClient  = maxFrameCountPerChunkIn;
        }

        float* ppSamplesFromClient[MAL_MAX_CHANNELS];
        for (mal_uint32 iChannel = 0; iChannel < pSRC->config.channels; ++iChannel) {
            ppSamplesFromClient[iChannel] = pSRC->linear.input[iChannel] + pSRC->linear.leftoverFrames;
        }
        
        mal_uint32 framesReadFromClient = 0;
        if (framesToReadFromClient > pSRC->linear.leftoverFrames) {
            framesReadFromClient = (mal_uint32)pSRC->config.onReadDeinterleaved(pSRC, (mal_uint32)framesToReadFromClient - pSRC->linear.leftoverFrames, (void**)ppSamplesFromClient, pUserData);
        }

        framesReadFromClient += pSRC->linear.leftoverFrames;  // <-- You can sort of think of it as though we've re-read the leftover samples from the client.
        if (framesReadFromClient < 2) {
            break;
        }

        for (mal_uint32 iChannel = 0; iChannel < pSRC->config.channels; ++iChannel) {
            ppSamplesFromClient[iChannel] = pSRC->linear.input[iChannel];
        }


        // Write Output Data
        // =================

        // At this point we have a bunch of frames that the client has given to us for processing. From this we can determine the maximum number of output frames
        // that can be processed from this input. We want to output as many samples as possible from our input data.
        float tAvailable = framesReadFromClient - tBeg - 1; // Subtract 1 because the last input sample is needed for interpolation and cannot be included in the output sample count calculation.

        mal_uint32 maxOutputFramesToRead = (mal_uint32)(tAvailable / factor);
        if (maxOutputFramesToRead == 0) {
            maxOutputFramesToRead = 1;
        }
        if (maxOutputFramesToRead > framesToRead) {
            maxOutputFramesToRead = (mal_uint32)framesToRead;
        }

        // Output frames are always read in groups of 4 because I'm planning on using this as a reference for some SIMD-y stuff later.
        mal_uint32 maxOutputFramesToRead4 = maxOutputFramesToRead/4;
        for (mal_uint32 iChannel = 0; iChannel < pSRC->config.channels; ++iChannel) {
            float t0 = pSRC->linear.timeIn + factor*0;
            float t1 = pSRC->linear.timeIn + factor*1;
            float t2 = pSRC->linear.timeIn + factor*2;
            float t3 = pSRC->linear.timeIn + factor*3;

            for (mal_uint32 iFrameOut = 0; iFrameOut < maxOutputFramesToRead4; iFrameOut += 1) {
                float iPrevSample0 = (float)floor(t0);
                float iPrevSample1 = (float)floor(t1);
                float iPrevSample2 = (float)floor(t2);
                float iPrevSample3 = (float)floor(t3);
                
                float iNextSample0 = iPrevSample0 + 1;
                float iNextSample1 = iPrevSample1 + 1;
                float iNextSample2 = iPrevSample2 + 1;
                float iNextSample3 = iPrevSample3 + 1;

                float alpha0 = t0 - iPrevSample0;
                float alpha1 = t1 - iPrevSample1;
                float alpha2 = t2 - iPrevSample2;
                float alpha3 = t3 - iPrevSample3;

                float prevSample0 = ppSamplesFromClient[iChannel][(mal_uint32)iPrevSample0];
                float prevSample1 = ppSamplesFromClient[iChannel][(mal_uint32)iPrevSample1];
                float prevSample2 = ppSamplesFromClient[iChannel][(mal_uint32)iPrevSample2];
                float prevSample3 = ppSamplesFromClient[iChannel][(mal_uint32)iPrevSample3];
                
                float nextSample0 = ppSamplesFromClient[iChannel][(mal_uint32)iNextSample0];
                float nextSample1 = ppSamplesFromClient[iChannel][(mal_uint32)iNextSample1];
                float nextSample2 = ppSamplesFromClient[iChannel][(mal_uint32)iNextSample2];
                float nextSample3 = ppSamplesFromClient[iChannel][(mal_uint32)iNextSample3];

                ppNextSamplesOut[iChannel][iFrameOut*4 + 0] = mal_mix_f32_fast(prevSample0, nextSample0, alpha0);
                ppNextSamplesOut[iChannel][iFrameOut*4 + 1] = mal_mix_f32_fast(prevSample1, nextSample1, alpha1);
                ppNextSamplesOut[iChannel][iFrameOut*4 + 2] = mal_mix_f32_fast(prevSample2, nextSample2, alpha2);
                ppNextSamplesOut[iChannel][iFrameOut*4 + 3] = mal_mix_f32_fast(prevSample3, nextSample3, alpha3);

                t0 += factor*4;
                t1 += factor*4;
                t2 += factor*4;
                t3 += factor*4;
            }

            float t = pSRC->linear.timeIn + (factor*maxOutputFramesToRead4*4);
            for (mal_uint32 iFrameOut = (maxOutputFramesToRead4*4); iFrameOut < maxOutputFramesToRead; iFrameOut += 1) {
                float iPrevSample = (float)floor(t);
                float iNextSample = iPrevSample + 1;
                float alpha = t - iPrevSample;

                mal_assert(iPrevSample < mal_countof(pSRC->linear.input[iChannel]));
                mal_assert(iNextSample < mal_countof(pSRC->linear.input[iChannel]));

                float prevSample = ppSamplesFromClient[iChannel][(mal_uint32)iPrevSample];
                float nextSample = ppSamplesFromClient[iChannel][(mal_uint32)iNextSample];

                ppNextSamplesOut[iChannel][iFrameOut] = mal_mix_f32_fast(prevSample, nextSample, alpha);

                t += factor;
            }

            ppNextSamplesOut[iChannel] += maxOutputFramesToRead;
        }

        totalFramesRead += maxOutputFramesToRead;


        // Residual
        // ========
        float tNext = pSRC->linear.timeIn + (maxOutputFramesToRead*factor);

        pSRC->linear.timeIn = tNext;
        mal_assert(tNext <= framesReadFromClient+1);

        mal_uint32 iNextFrame = (mal_uint32)floor(tNext);
        pSRC->linear.leftoverFrames = framesReadFromClient - iNextFrame;
        pSRC->linear.timeIn = tNext - iNextFrame;

        for (mal_uint32 iChannel = 0; iChannel < pSRC->config.channels; ++iChannel) {
            for (mal_uint32 iFrame = 0; iFrame < pSRC->linear.leftoverFrames; ++iFrame) {
                float sample = ppSamplesFromClient[iChannel][framesReadFromClient-pSRC->linear.leftoverFrames + iFrame];
                ppSamplesFromClient[iChannel][iFrame] = sample;
            }
        }

        
        // Exit the loop if we've found everything from the client.
        if (framesReadFromClient < framesToReadFromClient) {
            break;
        }
    }

    return totalFramesRead;
}


mal_src_config mal_src_config_init_new()
{
    mal_src_config config;
    mal_zero_object(&config);

    return config;
}

mal_src_config mal_src_config_init(mal_uint32 sampleRateIn, mal_uint32 sampleRateOut, mal_uint32 channels, mal_src_read_deinterleaved_proc onReadDeinterleaved, void* pUserData)
{
    mal_src_config config = mal_src_config_init_new();
    config.sampleRateIn = sampleRateIn;
    config.sampleRateOut = sampleRateOut;
    config.channels = channels;
    config.onReadDeinterleaved = onReadDeinterleaved;
    config.pUserData = pUserData;

    return config;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Sinc Sample Rate Conversion
// ===========================
//
// The sinc SRC algorithm uses a windowed sinc to perform interpolation of samples. Currently, mini_al's implementation supports rectangular and Hann window
// methods.
//
// Whenever an output sample is being computed, it looks at a sub-section of the input samples. I've called this sub-section in the code below the "window",
// which I realize is a bit ambigous with the mathematical "window", but it works for me when I need to conceptualize things in my head. The window is made up
// of two halves. The first half contains past input samples (initialized to zero), and the second half contains future input samples. As time moves forward
// and input samples are consumed, the window moves forward. The larger the window, the better the quality at the expense of slower processing. The window is
// limited the range [MAL_SRC_SINC_MIN_WINDOW_WIDTH, MAL_SRC_SINC_MAX_WINDOW_WIDTH] and defaults to MAL_SRC_SINC_DEFAULT_WINDOW_WIDTH.
//
// Input samples are cached for efficiency (to prevent frequently requesting tiny numbers of samples from the client). When the window gets to the end of the
// cache, it's moved back to the start, and more samples are read from the client. If the client has no more data to give, the cache is filled with zeros and
// the last of the input samples will be consumed. Once the last of the input samples have been consumed, no more samples will be output.
//
//
// When reading output samples, we always first read whatever is already in the input cache. Only when the cache has been fully consumed do we read more data
// from the client.
//
// To access samples in the input buffer you do so relative to the window. When the window itself is at position 0, the first item in the buffer is accessed
// with "windowPos + windowWidth". Generally, to access any sample relative to the window you do "windowPos + windowWidth + sampleIndexRelativeToWindow".
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Comment this to disable interpolation of table lookups. Less accurate, but faster.
#define MAL_USE_SINC_TABLE_INTERPOLATION

// Retrieves a sample from the input buffer's window. Values >= 0 retrieve future samples. Negative values return past samples.
static MAL_INLINE float mal_src_sinc__get_input_sample_from_window(const mal_src* pSRC, mal_uint32 channel, mal_uint32 windowPosInSamples, mal_int32 sampleIndex)
{
    mal_assert(pSRC != NULL);
    mal_assert(channel < pSRC->config.channels);
    mal_assert(sampleIndex >= -(mal_int32)pSRC->config.sinc.windowWidth);
    mal_assert(sampleIndex <   (mal_int32)pSRC->config.sinc.windowWidth);

    // The window should always be contained within the input cache.
    mal_assert(windowPosInSamples <  mal_countof(pSRC->sinc.input[0]) - pSRC->config.sinc.windowWidth);
    
    return pSRC->sinc.input[channel][windowPosInSamples + pSRC->config.sinc.windowWidth + sampleIndex];
}

static MAL_INLINE float mal_src_sinc__interpolation_factor(const mal_src* pSRC, float x)
{
    mal_assert(pSRC != NULL);

    float xabs = (float)fabs(x);
    //if (xabs >= MAL_SRC_SINC_MAX_WINDOW_WIDTH /*pSRC->config.sinc.windowWidth*/) {
    //    xabs = 1;   // <-- A non-zero integer will always return 0.
    //}

    xabs = xabs * MAL_SRC_SINC_LOOKUP_TABLE_RESOLUTION;
    mal_int32 ixabs = (mal_int32)xabs;

#if defined(MAL_USE_SINC_TABLE_INTERPOLATION)
    float a = xabs - ixabs;
    return mal_mix_f32_fast(pSRC->sinc.table[ixabs], pSRC->sinc.table[ixabs+1], a);
#else
    return pSRC->sinc.table[ixabs];
#endif
}

#if defined(MAL_SUPPORT_SSE2)
static MAL_INLINE __m128 mal_fabsf_sse2(__m128 x)
{
    return _mm_and_ps(_mm_castsi128_ps(_mm_set1_epi32(0x7FFFFFFF)), x);
}

static MAL_INLINE __m128 mal_truncf_sse2(__m128 x)
{
    return _mm_cvtepi32_ps(_mm_cvttps_epi32(x));
}

static MAL_INLINE __m128 mal_src_sinc__interpolation_factor__sse2(const mal_src* pSRC, __m128 x)
{
    //__m128 windowWidth128 = _mm_set1_ps(MAL_SRC_SINC_MAX_WINDOW_WIDTH);
    __m128 resolution128  = _mm_set1_ps(MAL_SRC_SINC_LOOKUP_TABLE_RESOLUTION);
    //__m128 one            = _mm_set1_ps(1);

    __m128 xabs = mal_fabsf_sse2(x);

    // if (MAL_SRC_SINC_MAX_WINDOW_WIDTH <= xabs) xabs = 1 else xabs = xabs;
    //__m128 xcmp = _mm_cmp_ps(windowWidth128, xabs, 2);                      // 2 = Less than or equal = _mm_cmple_ps.
    //xabs = _mm_or_ps(_mm_and_ps(one, xcmp), _mm_andnot_ps(xcmp, xabs));     // xabs = (xcmp) ? 1 : xabs;

    xabs = _mm_mul_ps(xabs, resolution128);
    __m128i ixabs = _mm_cvttps_epi32(xabs);

    int* ixabsv = (int*)&ixabs;
    
    __m128 lo = _mm_set_ps(
        pSRC->sinc.table[ixabsv[3]],
        pSRC->sinc.table[ixabsv[2]],
        pSRC->sinc.table[ixabsv[1]],
        pSRC->sinc.table[ixabsv[0]]
    );

    __m128 hi = _mm_set_ps(
        pSRC->sinc.table[ixabsv[3]+1],
        pSRC->sinc.table[ixabsv[2]+1],
        pSRC->sinc.table[ixabsv[1]+1],
        pSRC->sinc.table[ixabsv[0]+1]
    );

    __m128 a = _mm_sub_ps(xabs, _mm_cvtepi32_ps(ixabs));
    __m128 r = mal_mix_f32_fast__sse2(lo, hi, a);

    return r;
}
#endif

#if defined(MAL_SUPPORT_AVX2)
static MAL_INLINE __m256 mal_fabsf_avx2(__m256 x)
{
    return _mm256_and_ps(_mm256_castsi256_ps(_mm256_set1_epi32(0x7FFFFFFF)), x);
}

#if 0
static MAL_INLINE __m256 mal_src_sinc__interpolation_factor__avx2(const mal_src* pSRC, __m256 x)
{
    //__m256 windowWidth256 = _mm256_set1_ps(MAL_SRC_SINC_MAX_WINDOW_WIDTH);
    __m256 resolution256  = _mm256_set1_ps(MAL_SRC_SINC_LOOKUP_TABLE_RESOLUTION);
    //__m256 one            = _mm256_set1_ps(1);

    __m256 xabs = mal_fabsf_avx2(x);

    // if (MAL_SRC_SINC_MAX_WINDOW_WIDTH <= xabs) xabs = 1 else xabs = xabs;
    //__m256 xcmp = _mm256_cmp_ps(windowWidth256, xabs, 2);                      // 2 = Less than or equal = _mm_cmple_ps.
    //xabs = _mm256_or_ps(_mm256_and_ps(one, xcmp), _mm256_andnot_ps(xcmp, xabs));     // xabs = (xcmp) ? 1 : xabs;

    xabs = _mm256_mul_ps(xabs, resolution256);

    __m256i ixabs = _mm256_cvttps_epi32(xabs);
    __m256 a = _mm256_sub_ps(xabs, _mm256_cvtepi32_ps(ixabs));

    
    int* ixabsv = (int*)&ixabs;

    __m256 lo = _mm256_set_ps(
        pSRC->sinc.table[ixabsv[7]],
        pSRC->sinc.table[ixabsv[6]],
        pSRC->sinc.table[ixabsv[5]],
        pSRC->sinc.table[ixabsv[4]],
        pSRC->sinc.table[ixabsv[3]],
        pSRC->sinc.table[ixabsv[2]],
        pSRC->sinc.table[ixabsv[1]],
        pSRC->sinc.table[ixabsv[0]]
    );
    
    __m256 hi = _mm256_set_ps(
        pSRC->sinc.table[ixabsv[7]+1],
        pSRC->sinc.table[ixabsv[6]+1],
        pSRC->sinc.table[ixabsv[5]+1],
        pSRC->sinc.table[ixabsv[4]+1],
        pSRC->sinc.table[ixabsv[3]+1],
        pSRC->sinc.table[ixabsv[2]+1],
        pSRC->sinc.table[ixabsv[1]+1],
        pSRC->sinc.table[ixabsv[0]+1]
    );

    __m256 r = mal_mix_f32_fast__avx2(lo, hi, a);

    return r;
}
#endif

#endif

#if defined(MAL_SUPPORT_NEON)
static MAL_INLINE float32x4_t mal_fabsf_neon(float32x4_t x)
{
    return vabdq_f32(vmovq_n_f32(0), x);
}

static MAL_INLINE float32x4_t mal_src_sinc__interpolation_factor__neon(const mal_src* pSRC, float32x4_t x)
{
    float32x4_t xabs = mal_fabsf_neon(x);
    xabs = vmulq_n_f32(xabs, MAL_SRC_SINC_LOOKUP_TABLE_RESOLUTION);

    int32x4_t ixabs = vcvtq_s32_f32(xabs);

    int* ixabsv = (int*)&ixabs;
    
    float lo[4];
    lo[0] = pSRC->sinc.table[ixabsv[0]];
    lo[1] = pSRC->sinc.table[ixabsv[1]];
    lo[2] = pSRC->sinc.table[ixabsv[2]];
    lo[3] = pSRC->sinc.table[ixabsv[3]];

    float hi[4];
    hi[0] = pSRC->sinc.table[ixabsv[0]+1];
    hi[1] = pSRC->sinc.table[ixabsv[1]+1];
    hi[2] = pSRC->sinc.table[ixabsv[2]+1];
    hi[3] = pSRC->sinc.table[ixabsv[3]+1];

    float32x4_t a = vsubq_f32(xabs, vcvtq_f32_s32(ixabs));
    float32x4_t r = mal_mix_f32_fast__neon(vld1q_f32(lo), vld1q_f32(hi), a);

    return r;
}
#endif

mal_uint64 mal_src_read_deinterleaved__sinc(mal_src* pSRC, mal_uint64 frameCount, void** ppSamplesOut, void* pUserData)
{
    mal_assert(pSRC != NULL);
    mal_assert(frameCount > 0);
    mal_assert(ppSamplesOut != NULL);

    float factor = (float)pSRC->config.sampleRateIn / pSRC->config.sampleRateOut;
    float inverseFactor = 1/factor;

    mal_int32 windowWidth  = (mal_int32)pSRC->config.sinc.windowWidth;
    mal_int32 windowWidth2 = windowWidth*2;

    // There are cases where it's actually more efficient to increase the window width so that it's aligned with the respective
    // SIMD pipeline being used.
    mal_int32 windowWidthSIMD = windowWidth;
    if (pSRC->useNEON) {
        windowWidthSIMD = (windowWidthSIMD + 1) & ~(1);
    } else  if (pSRC->useAVX512) {
        windowWidthSIMD = (windowWidthSIMD + 7) & ~(7);
    } else if (pSRC->useAVX2) {
        windowWidthSIMD = (windowWidthSIMD + 3) & ~(3);
    } else if (pSRC->useSSE2) {
        windowWidthSIMD = (windowWidthSIMD + 1) & ~(1);
    }

    mal_int32 windowWidthSIMD2 = windowWidthSIMD*2;
    (void)windowWidthSIMD2; // <-- Silence a warning when SIMD is disabled.

    float* ppNextSamplesOut[MAL_MAX_CHANNELS];
    mal_copy_memory(ppNextSamplesOut, ppSamplesOut, sizeof(void*) * pSRC->config.channels);

    float _windowSamplesUnaligned[MAL_SRC_SINC_MAX_WINDOW_WIDTH*2 + MAL_SIMD_ALIGNMENT];
    float* windowSamples = (float*)(((mal_uintptr)_windowSamplesUnaligned + MAL_SIMD_ALIGNMENT-1) & ~(MAL_SIMD_ALIGNMENT-1));
    mal_zero_memory(windowSamples, MAL_SRC_SINC_MAX_WINDOW_WIDTH*2 * sizeof(float));

    float _iWindowFUnaligned[MAL_SRC_SINC_MAX_WINDOW_WIDTH*2 + MAL_SIMD_ALIGNMENT];
    float* iWindowF = (float*)(((mal_uintptr)_iWindowFUnaligned + MAL_SIMD_ALIGNMENT-1) & ~(MAL_SIMD_ALIGNMENT-1));
    mal_zero_memory(iWindowF, MAL_SRC_SINC_MAX_WINDOW_WIDTH*2 * sizeof(float));
    for (mal_int32 i = 0; i < windowWidth2; ++i) {
        iWindowF[i] = (float)(i - windowWidth);
    }

    mal_uint64 totalOutputFramesRead = 0;
    while (totalOutputFramesRead < frameCount) {
        // The maximum number of frames we can read this iteration depends on how many input samples we have available to us. This is the number
        // of input samples between the end of the window and the end of the cache.
        mal_uint32 maxInputSamplesAvailableInCache = mal_countof(pSRC->sinc.input[0]) - (pSRC->config.sinc.windowWidth*2) - pSRC->sinc.windowPosInSamples;
        if (maxInputSamplesAvailableInCache > pSRC->sinc.inputFrameCount) {
            maxInputSamplesAvailableInCache = pSRC->sinc.inputFrameCount;
        }

        // Never consume the tail end of the input data if requested.
        if (pSRC->config.neverConsumeEndOfInput) {
            if (maxInputSamplesAvailableInCache >= pSRC->config.sinc.windowWidth) {
                maxInputSamplesAvailableInCache -= pSRC->config.sinc.windowWidth;
            } else {
                maxInputSamplesAvailableInCache  = 0;
            }
        }

        float timeInBeg = pSRC->sinc.timeIn;
        float timeInEnd = (float)(pSRC->sinc.windowPosInSamples + maxInputSamplesAvailableInCache);

        mal_assert(timeInBeg >= 0);
        mal_assert(timeInBeg <= timeInEnd);

        mal_uint64 maxOutputFramesToRead = (mal_uint64)(((timeInEnd - timeInBeg) * inverseFactor));

        mal_uint64 outputFramesRemaining = frameCount - totalOutputFramesRead;
        mal_uint64 outputFramesToRead = outputFramesRemaining;
        if (outputFramesToRead > maxOutputFramesToRead) {
            outputFramesToRead = maxOutputFramesToRead;
        }

        for (mal_uint32 iChannel = 0; iChannel < pSRC->config.channels; iChannel += 1) {
            // Do SRC.
            float timeIn = timeInBeg;
            for (mal_uint32 iSample = 0; iSample < outputFramesToRead; iSample += 1) {
                float sampleOut    = 0;
                float iTimeInF     = mal_floorf(timeIn);
                mal_uint32 iTimeIn = (mal_uint32)iTimeInF;

                mal_int32 iWindow = 0;

                // Pre-load the window samples into an aligned buffer to begin with. Need to put these into an aligned buffer to make SIMD easier.
                windowSamples[0] = 0;   // <-- The first sample is always zero.
                for (mal_int32 i = 1; i < windowWidth2; ++i) {
                    windowSamples[i] = pSRC->sinc.input[iChannel][iTimeIn + i];
                }

#if defined(MAL_SUPPORT_AVX2) || defined(MAL_SUPPORT_AVX512)
                if (pSRC->useAVX2 || pSRC->useAVX512) {
                    __m256i ixabs[MAL_SRC_SINC_MAX_WINDOW_WIDTH*2/8];
                    __m256      a[MAL_SRC_SINC_MAX_WINDOW_WIDTH*2/8];
                    __m256 resolution256 = _mm256_set1_ps(MAL_SRC_SINC_LOOKUP_TABLE_RESOLUTION);

                    __m256 t = _mm256_set1_ps((timeIn - iTimeInF));
                    __m256 r = _mm256_set1_ps(0);

                    mal_int32 windowWidth8 = windowWidthSIMD2 >> 3;
                    for (mal_int32 iWindow8 = 0; iWindow8 < windowWidth8; iWindow8 += 1) {
                        __m256 w = *((__m256*)iWindowF + iWindow8);

                        __m256 xabs = _mm256_sub_ps(t, w);
                        xabs = mal_fabsf_avx2(xabs);
                        xabs = _mm256_mul_ps(xabs, resolution256);

                        ixabs[iWindow8] = _mm256_cvttps_epi32(xabs);
                            a[iWindow8] = _mm256_sub_ps(xabs, _mm256_cvtepi32_ps(ixabs[iWindow8]));
                    }
                    
                    for (mal_int32 iWindow8 = 0; iWindow8 < windowWidth8; iWindow8 += 1) {
                        int* ixabsv = (int*)&ixabs[iWindow8];

                        __m256 lo = _mm256_set_ps(
                            pSRC->sinc.table[ixabsv[7]],
                            pSRC->sinc.table[ixabsv[6]],
                            pSRC->sinc.table[ixabsv[5]],
                            pSRC->sinc.table[ixabsv[4]],
                            pSRC->sinc.table[ixabsv[3]],
                            pSRC->sinc.table[ixabsv[2]],
                            pSRC->sinc.table[ixabsv[1]],
                            pSRC->sinc.table[ixabsv[0]]
                        );
    
                        __m256 hi = _mm256_set_ps(
                            pSRC->sinc.table[ixabsv[7]+1],
                            pSRC->sinc.table[ixabsv[6]+1],
                            pSRC->sinc.table[ixabsv[5]+1],
                            pSRC->sinc.table[ixabsv[4]+1],
                            pSRC->sinc.table[ixabsv[3]+1],
                            pSRC->sinc.table[ixabsv[2]+1],
                            pSRC->sinc.table[ixabsv[1]+1],
                            pSRC->sinc.table[ixabsv[0]+1]
                        );

                        __m256 s = *((__m256*)windowSamples + iWindow8);
                        r = _mm256_add_ps(r, _mm256_mul_ps(s, mal_mix_f32_fast__avx2(lo, hi, a[iWindow8])));
                    }

                    // Horizontal add.
                    __m256 x = _mm256_hadd_ps(r, _mm256_permute2f128_ps(r, r, 1));
                           x = _mm256_hadd_ps(x, x);
                           x = _mm256_hadd_ps(x, x);
                    sampleOut += _mm_cvtss_f32(_mm256_castps256_ps128(x));

                    iWindow += windowWidth8 * 8;
                }
                else
#endif
#if defined(MAL_SUPPORT_SSE2)
                if (pSRC->useSSE2) {
                    __m128 t = _mm_set1_ps((timeIn - iTimeInF));
                    __m128 r = _mm_set1_ps(0);

                    mal_int32 windowWidth4 = windowWidthSIMD2 >> 2;
                    for (mal_int32 iWindow4 = 0; iWindow4 < windowWidth4; iWindow4 += 1) {
                        __m128* s = (__m128*)windowSamples + iWindow4;
                        __m128* w = (__m128*)iWindowF + iWindow4;

                        __m128 a = mal_src_sinc__interpolation_factor__sse2(pSRC, _mm_sub_ps(t, *w));
                        r = _mm_add_ps(r, _mm_mul_ps(*s, a));
                    }

                    sampleOut += ((float*)(&r))[0];
                    sampleOut += ((float*)(&r))[1];
                    sampleOut += ((float*)(&r))[2];
                    sampleOut += ((float*)(&r))[3];

                    iWindow += windowWidth4 * 4;
                }
                else
#endif
#if defined(MAL_SUPPORT_NEON)
                if (pSRC->useNEON) {
                    float32x4_t t = vmovq_n_f32((timeIn - iTimeInF));
                    float32x4_t r = vmovq_n_f32(0);

                    mal_int32 windowWidth4 = windowWidthSIMD2 >> 2;
                    for (mal_int32 iWindow4 = 0; iWindow4 < windowWidth4; iWindow4 += 1) {
                        float32x4_t* s = (float32x4_t*)windowSamples + iWindow4;
                        float32x4_t* w = (float32x4_t*)iWindowF + iWindow4;

                        float32x4_t a = mal_src_sinc__interpolation_factor__neon(pSRC, vsubq_f32(t, *w));
                        r = vaddq_f32(r, vmulq_f32(*s, a));
                    }

                    sampleOut += ((float*)(&r))[0];
                    sampleOut += ((float*)(&r))[1];
                    sampleOut += ((float*)(&r))[2];
                    sampleOut += ((float*)(&r))[3];

                    iWindow += windowWidth4 * 4;
                }
                else
#endif
                {
                    iWindow += 1;   // The first one is a dummy for SIMD alignment purposes. Skip it.
                }

                // Non-SIMD/Reference implementation. 
                float t = (timeIn - iTimeIn);
                for (; iWindow < windowWidth2; iWindow += 1) {
                    float s = windowSamples[iWindow];
                    float w = iWindowF[iWindow];

                    float a = mal_src_sinc__interpolation_factor(pSRC, (t - w));
                    float r = s * a;

                    sampleOut += r;
                }

                ppNextSamplesOut[iChannel][iSample] = (float)sampleOut;

                timeIn += factor;
            }

            ppNextSamplesOut[iChannel] += outputFramesToRead;
        }

        totalOutputFramesRead += outputFramesToRead;

        mal_uint32 prevWindowPosInSamples = pSRC->sinc.windowPosInSamples;

        pSRC->sinc.timeIn            += (outputFramesToRead * factor);
        pSRC->sinc.windowPosInSamples = (mal_uint32)pSRC->sinc.timeIn;
        pSRC->sinc.inputFrameCount   -= pSRC->sinc.windowPosInSamples - prevWindowPosInSamples;

        // If the window has reached a point where we cannot read a whole output sample it needs to be moved back to the start.
        mal_uint32 availableOutputFrames = (mal_uint32)((timeInEnd - pSRC->sinc.timeIn) * inverseFactor);

        if (availableOutputFrames == 0) {
            size_t samplesToMove = mal_countof(pSRC->sinc.input[0]) - pSRC->sinc.windowPosInSamples;

            pSRC->sinc.timeIn            -= mal_floorf(pSRC->sinc.timeIn);
            pSRC->sinc.windowPosInSamples = 0;

            // Move everything from the end of the cache up to the front.
            for (mal_uint32 iChannel = 0; iChannel < pSRC->config.channels; iChannel += 1) {
                memmove(pSRC->sinc.input[iChannel], pSRC->sinc.input[iChannel] + mal_countof(pSRC->sinc.input[iChannel]) - samplesToMove, samplesToMove * sizeof(*pSRC->sinc.input[iChannel]));
            }
        }

        // Read more data from the client if required.
        if (pSRC->isEndOfInputLoaded) {
            pSRC->isEndOfInputLoaded = MAL_FALSE;
            break;
        }

        // Everything beyond this point is reloading. If we're at the end of the input data we do _not_ want to try reading any more in this function call. If the
        // caller wants to keep trying, they can reload their internal data sources and call this function again. We should never be 
        mal_assert(pSRC->isEndOfInputLoaded == MAL_FALSE);

        if (pSRC->sinc.inputFrameCount <= pSRC->config.sinc.windowWidth || availableOutputFrames == 0) {
            float* ppInputDst[MAL_MAX_CHANNELS] = {0};
            for (mal_uint32 iChannel = 0; iChannel < pSRC->config.channels; iChannel += 1) {
                ppInputDst[iChannel] = pSRC->sinc.input[iChannel] + pSRC->config.sinc.windowWidth + pSRC->sinc.inputFrameCount;
            }

            // Now read data from the client.
            mal_uint32 framesToReadFromClient = mal_countof(pSRC->sinc.input[0]) - (pSRC->config.sinc.windowWidth + pSRC->sinc.inputFrameCount);

            mal_uint32 framesReadFromClient = 0;
            if (framesToReadFromClient > 0) {
                framesReadFromClient = pSRC->config.onReadDeinterleaved(pSRC, framesToReadFromClient, (void**)ppInputDst, pUserData);
            }

            if (framesReadFromClient != framesToReadFromClient) {
                pSRC->isEndOfInputLoaded = MAL_TRUE;
            } else {
                pSRC->isEndOfInputLoaded = MAL_FALSE;
            }

            if (framesReadFromClient != 0) {
                pSRC->sinc.inputFrameCount += framesReadFromClient;
            } else {
                // We couldn't get anything more from the client. If no more output samples can be computed from the available input samples
                // we need to return.
                if (pSRC->config.neverConsumeEndOfInput) {
                    if ((pSRC->sinc.inputFrameCount * inverseFactor) <= pSRC->config.sinc.windowWidth) {
                        break;
                    }
                } else {
                    if ((pSRC->sinc.inputFrameCount * inverseFactor) < 1) {
                        break;
                    }
                }
            }

            // Anything left over in the cache must be set to zero.
            mal_uint32 leftoverFrames = mal_countof(pSRC->sinc.input[0]) - (pSRC->config.sinc.windowWidth + pSRC->sinc.inputFrameCount);
            if (leftoverFrames > 0) {
                for (mal_uint32 iChannel = 0; iChannel < pSRC->config.channels; iChannel += 1) {
                    mal_zero_memory(pSRC->sinc.input[iChannel] + pSRC->config.sinc.windowWidth + pSRC->sinc.inputFrameCount, leftoverFrames * sizeof(float));
                }
            }
        }
    }

    return totalOutputFramesRead;
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// FORMAT CONVERSION
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void mal_pcm_convert(void* pOut, mal_format formatOut, const void* pIn, mal_format formatIn, mal_uint64 sampleCount, mal_dither_mode ditherMode)
{
    if (formatOut == formatIn) {
        mal_copy_memory_64(pOut, pIn, sampleCount * mal_get_bytes_per_sample(formatOut));
        return;
    }

    switch (formatIn)
    {
        case mal_format_u8:
        {
            switch (formatOut)
            {
                case mal_format_s16: mal_pcm_u8_to_s16(pOut, pIn, sampleCount, ditherMode); return;
                case mal_format_s24: mal_pcm_u8_to_s24(pOut, pIn, sampleCount, ditherMode); return;
                case mal_format_s32: mal_pcm_u8_to_s32(pOut, pIn, sampleCount, ditherMode); return;
                case mal_format_f32: mal_pcm_u8_to_f32(pOut, pIn, sampleCount, ditherMode); return;
                default: break;
            }
        } break;

        case mal_format_s16:
        {
            switch (formatOut)
            {
                case mal_format_u8:  mal_pcm_s16_to_u8( pOut, pIn, sampleCount, ditherMode); return;
                case mal_format_s24: mal_pcm_s16_to_s24(pOut, pIn, sampleCount, ditherMode); return;
                case mal_format_s32: mal_pcm_s16_to_s32(pOut, pIn, sampleCount, ditherMode); return;
                case mal_format_f32: mal_pcm_s16_to_f32(pOut, pIn, sampleCount, ditherMode); return;
                default: break;
            }
        } break;

        case mal_format_s24:
        {
            switch (formatOut)
            {
                case mal_format_u8:  mal_pcm_s24_to_u8( pOut, pIn, sampleCount, ditherMode); return;
                case mal_format_s16: mal_pcm_s24_to_s16(pOut, pIn, sampleCount, ditherMode); return;
                case mal_format_s32: mal_pcm_s24_to_s32(pOut, pIn, sampleCount, ditherMode); return;
                case mal_format_f32: mal_pcm_s24_to_f32(pOut, pIn, sampleCount, ditherMode); return;
                default: break;
            }
        } break;

        case mal_format_s32:
        {
            switch (formatOut)
            {
                case mal_format_u8:  mal_pcm_s32_to_u8( pOut, pIn, sampleCount, ditherMode); return;
                case mal_format_s16: mal_pcm_s32_to_s16(pOut, pIn, sampleCount, ditherMode); return;
                case mal_format_s24: mal_pcm_s32_to_s24(pOut, pIn, sampleCount, ditherMode); return;
                case mal_format_f32: mal_pcm_s32_to_f32(pOut, pIn, sampleCount, ditherMode); return;
                default: break;
            }
        } break;

        case mal_format_f32:
        {
            switch (formatOut)
            {
                case mal_format_u8:  mal_pcm_f32_to_u8( pOut, pIn, sampleCount, ditherMode); return;
                case mal_format_s16: mal_pcm_f32_to_s16(pOut, pIn, sampleCount, ditherMode); return;
                case mal_format_s24: mal_pcm_f32_to_s24(pOut, pIn, sampleCount, ditherMode); return;
                case mal_format_s32: mal_pcm_f32_to_s32(pOut, pIn, sampleCount, ditherMode); return;
                default: break;
            }
        } break;

        default: break;
    }
}

void mal_deinterleave_pcm_frames(mal_format format, mal_uint32 channels, mal_uint32 frameCount, const void* pInterleavedPCMFrames, void** ppDeinterleavedPCMFrames)
{
    if (pInterleavedPCMFrames == NULL || ppDeinterleavedPCMFrames == NULL) {
        return; // Invalid args.
    }

    // For efficiency we do this per format.
    switch (format) {
        case mal_format_s16:
        {
            const mal_int16* pSrcS16 = (const mal_int16*)pInterleavedPCMFrames;
            for (mal_uint32 iPCMFrame = 0; iPCMFrame < frameCount; ++iPCMFrame) {
                for (mal_uint32 iChannel = 0; iChannel < channels; ++iChannel) {
                    mal_int16* pDstS16 = (mal_int16*)ppDeinterleavedPCMFrames[iChannel];
                    pDstS16[iPCMFrame] = pSrcS16[iPCMFrame*channels+iChannel];
                }
            }
        } break;
        
        case mal_format_f32:
        {
            const float* pSrcF32 = (const float*)pInterleavedPCMFrames;
            for (mal_uint32 iPCMFrame = 0; iPCMFrame < frameCount; ++iPCMFrame) {
                for (mal_uint32 iChannel = 0; iChannel < channels; ++iChannel) {
                    float* pDstF32 = (float*)ppDeinterleavedPCMFrames[iChannel];
                    pDstF32[iPCMFrame] = pSrcF32[iPCMFrame*channels+iChannel];
                }
            }
        } break;
        
        default:
        {
            mal_uint32 sampleSizeInBytes = mal_get_bytes_per_sample(format);

            for (mal_uint32 iPCMFrame = 0; iPCMFrame < frameCount; ++iPCMFrame) {
                for (mal_uint32 iChannel = 0; iChannel < channels; ++iChannel) {
                          void* pDst = mal_offset_ptr(ppDeinterleavedPCMFrames[iChannel], iPCMFrame*sampleSizeInBytes);
                    const void* pSrc = mal_offset_ptr(pInterleavedPCMFrames, (iPCMFrame*channels+iChannel)*sampleSizeInBytes);
                    memcpy(pDst, pSrc, sampleSizeInBytes);
                }
            }
        } break;
    }
}

void mal_interleave_pcm_frames(mal_format format, mal_uint32 channels, mal_uint32 frameCount, const void** ppDeinterleavedPCMFrames, void* pInterleavedPCMFrames)
{
    switch (format)
    {
        case mal_format_s16:
        {
            mal_int16* pDstS16 = (mal_int16*)pInterleavedPCMFrames;
            for (mal_uint32 iPCMFrame = 0; iPCMFrame < frameCount; ++iPCMFrame) {
                for (mal_uint32 iChannel = 0; iChannel < channels; ++iChannel) {
                    const mal_int16* pSrcS16 = (const mal_int16*)ppDeinterleavedPCMFrames[iChannel];
                    pDstS16[iPCMFrame*channels+iChannel] = pSrcS16[iPCMFrame];
                }
            }
        } break;
        
        case mal_format_f32:
        {
            float* pDstF32 = (float*)pInterleavedPCMFrames;
            for (mal_uint32 iPCMFrame = 0; iPCMFrame < frameCount; ++iPCMFrame) {
                for (mal_uint32 iChannel = 0; iChannel < channels; ++iChannel) {
                    const float* pSrcF32 = (const float*)ppDeinterleavedPCMFrames[iChannel];
                    pDstF32[iPCMFrame*channels+iChannel] = pSrcF32[iPCMFrame];
                }
            }
        } break;
    
        default:
        {
            mal_uint32 sampleSizeInBytes = mal_get_bytes_per_sample(format);

            for (mal_uint32 iPCMFrame = 0; iPCMFrame < frameCount; ++iPCMFrame) {
                for (mal_uint32 iChannel = 0; iChannel < channels; ++iChannel) {
                          void* pDst = mal_offset_ptr(pInterleavedPCMFrames, (iPCMFrame*channels+iChannel)*sampleSizeInBytes);
                    const void* pSrc = mal_offset_ptr(ppDeinterleavedPCMFrames[iChannel], iPCMFrame*sampleSizeInBytes);
                    memcpy(pDst, pSrc, sampleSizeInBytes);
                }
            }
        } break;
    }
}



typedef struct
{
    mal_dsp* pDSP;
    void* pUserDataForClient;
} mal_dsp_callback_data;

mal_uint32 mal_dsp__pre_format_converter_on_read(mal_format_converter* pConverter, mal_uint32 frameCount, void* pFramesOut, void* pUserData)
{
    (void)pConverter;

    mal_dsp_callback_data* pData = (mal_dsp_callback_data*)pUserData;
    mal_assert(pData != NULL);

    mal_dsp* pDSP = pData->pDSP;
    mal_assert(pDSP != NULL);

    return pDSP->onRead(pDSP, frameCount, pFramesOut, pData->pUserDataForClient);
}

mal_uint32 mal_dsp__post_format_converter_on_read(mal_format_converter* pConverter, mal_uint32 frameCount, void* pFramesOut, void* pUserData)
{
    (void)pConverter;

    mal_dsp_callback_data* pData = (mal_dsp_callback_data*)pUserData;
    mal_assert(pData != NULL);

    mal_dsp* pDSP = pData->pDSP;
    mal_assert(pDSP != NULL);

    // When this version of this callback is used it means we're reading directly from the client.
    mal_assert(pDSP->isPreFormatConversionRequired == MAL_FALSE);
    mal_assert(pDSP->isChannelRoutingRequired == MAL_FALSE);
    mal_assert(pDSP->isSRCRequired == MAL_FALSE);

    return pDSP->onRead(pDSP, frameCount, pFramesOut, pData->pUserDataForClient);
}

mal_uint32 mal_dsp__post_format_converter_on_read_deinterleaved(mal_format_converter* pConverter, mal_uint32 frameCount, void** ppSamplesOut, void* pUserData)
{
    (void)pConverter;

    mal_dsp_callback_data* pData = (mal_dsp_callback_data*)pUserData;
    mal_assert(pData != NULL);

    mal_dsp* pDSP = pData->pDSP;
    mal_assert(pDSP != NULL);

    if (!pDSP->isChannelRoutingAtStart) {
        return (mal_uint32)mal_channel_router_read_deinterleaved(&pDSP->channelRouter, frameCount, ppSamplesOut, pUserData);
    } else {
        if (pDSP->isSRCRequired) {
            return (mal_uint32)mal_src_read_deinterleaved(&pDSP->src, frameCount, ppSamplesOut, pUserData);
        } else {
            return (mal_uint32)mal_channel_router_read_deinterleaved(&pDSP->channelRouter, frameCount, ppSamplesOut, pUserData);
        }
    }
}

mal_uint32 mal_dsp__src_on_read_deinterleaved(mal_src* pSRC, mal_uint32 frameCount, void** ppSamplesOut, void* pUserData)
{
    (void)pSRC;

    mal_dsp_callback_data* pData = (mal_dsp_callback_data*)pUserData;
    mal_assert(pData != NULL);

    mal_dsp* pDSP = pData->pDSP;
    mal_assert(pDSP != NULL);

    // If the channel routing stage is at the front we need to read from that. Otherwise we read from the pre format converter.
    if (pDSP->isChannelRoutingAtStart) {
        return (mal_uint32)mal_channel_router_read_deinterleaved(&pDSP->channelRouter, frameCount, ppSamplesOut, pUserData);
    } else {
        return (mal_uint32)mal_format_converter_read_deinterleaved(&pDSP->formatConverterIn, frameCount, ppSamplesOut, pUserData);
    }
}

mal_uint32 mal_dsp__channel_router_on_read_deinterleaved(mal_channel_router* pRouter, mal_uint32 frameCount, void** ppSamplesOut, void* pUserData)
{
    (void)pRouter;

    mal_dsp_callback_data* pData = (mal_dsp_callback_data*)pUserData;
    mal_assert(pData != NULL);

    mal_dsp* pDSP = pData->pDSP;
    mal_assert(pDSP != NULL);

    // If the channel routing stage is at the front of the pipeline we read from the pre format converter. Otherwise we read from the sample rate converter.
    if (pDSP->isChannelRoutingAtStart) {
        return (mal_uint32)mal_format_converter_read_deinterleaved(&pDSP->formatConverterIn, frameCount, ppSamplesOut, pUserData);
    } else {
        if (pDSP->isSRCRequired) {
            return (mal_uint32)mal_src_read_deinterleaved(&pDSP->src, frameCount, ppSamplesOut, pUserData);
        } else {
            return (mal_uint32)mal_format_converter_read_deinterleaved(&pDSP->formatConverterIn, frameCount, ppSamplesOut, pUserData);
        }
    }
}

mal_result mal_dsp_init(const mal_dsp_config* pConfig, mal_dsp* pDSP)
{
    if (pDSP == NULL) {
        return MAL_INVALID_ARGS;
    }

    mal_zero_object(pDSP);
    pDSP->onRead = pConfig->onRead;
    pDSP->pUserData = pConfig->pUserData;
    pDSP->isDynamicSampleRateAllowed = pConfig->allowDynamicSampleRate;


    // In general, this is the pipeline used for data conversion. Note that this can actually change which is explained later.
    //
    //   Pre Format Conversion -> Sample Rate Conversion -> Channel Routing -> Post Format Conversion
    //
    // Pre Format Conversion
    // ---------------------
    // This is where the sample data is converted to a format that's usable by the later stages in the pipeline. Input data
    // is converted to deinterleaved floating-point.
    //
    // Channel Routing
    // ---------------
    // Channel routing is where stereo is converted to 5.1, mono is converted to stereo, etc. This stage depends on the
    // pre format conversion stage.
    //
    // Sample Rate Conversion
    // ----------------------
    // Sample rate conversion depends on the pre format conversion stage and as the name implies performs sample rate conversion.
    //
    // Post Format Conversion
    // ----------------------
    // This stage is where our deinterleaved floating-point data from the previous stages are converted to the requested output
    // format.
    //
    //
    // Optimizations
    // -------------
    // Sometimes the conversion pipeline is rearranged for efficiency. The first obvious optimization is to eliminate unnecessary
    // stages in the pipeline. When no channel routing nor sample rate conversion is necessary, the entire pipeline is optimized
    // down to just this:
    //
    //   Post Format Conversion
    //
    // When sample rate conversion is not unnecessary:
    //
    //   Pre Format Conversion -> Channel Routing -> Post Format Conversion
    //
    // When channel routing is unnecessary:
    //
    //   Pre Format Conversion -> Sample Rate Conversion -> Post Format Conversion
    //
    // A slightly less obvious optimization is used depending on whether or not we are increasing or decreasing the number of
    // channels. Because everything in the pipeline works on a per-channel basis, the efficiency of the pipeline is directly
    // proportionate to the number of channels that need to be processed. Therefore, it's can be more efficient to move the
    // channel conversion stage to an earlier or later stage. When the channel count is being reduced, we move the channel
    // conversion stage to the start of the pipeline so that later stages can work on a smaller number of channels at a time.
    // Otherwise, we move the channel conversion stage to the end of the pipeline. When reducing the channel count, the pipeline
    // will look like this:
    //
    //   Pre Format Conversion -> Channel Routing -> Sample Rate Conversion -> Post Format Conversion
    //
    // Notice how the Channel Routing and Sample Rate Conversion stages are swapped so that the SRC stage has less data to process.

    // First we need to determine what's required and what's not.
    if (pConfig->sampleRateIn != pConfig->sampleRateOut || pConfig->allowDynamicSampleRate) {
        pDSP->isSRCRequired = MAL_TRUE;
    }
    if (pConfig->channelsIn != pConfig->channelsOut || !mal_channel_map_equal(pConfig->channelsIn, pConfig->channelMapIn, pConfig->channelMapOut)) {
        pDSP->isChannelRoutingRequired = MAL_TRUE;
    }

    // If neither a sample rate conversion nor channel conversion is necessary we can skip the pre format conversion.
    if (!pDSP->isSRCRequired && !pDSP->isChannelRoutingRequired) {
        // We don't need a pre format conversion stage, but we may still need a post format conversion stage.
        if (pConfig->formatIn != pConfig->formatOut) {
            pDSP->isPostFormatConversionRequired = MAL_TRUE;
        }
    } else {
        pDSP->isPreFormatConversionRequired  = MAL_TRUE;
        pDSP->isPostFormatConversionRequired = MAL_TRUE;
    }

    // Use a passthrough if none of the stages are being used.
    if (!pDSP->isPreFormatConversionRequired && !pDSP->isPostFormatConversionRequired && !pDSP->isChannelRoutingRequired && !pDSP->isSRCRequired) {
        pDSP->isPassthrough = MAL_TRUE;
    }

    // Move the channel conversion stage to the start of the pipeline if we are reducing the channel count.
    if (pConfig->channelsOut < pConfig->channelsIn) {
        pDSP->isChannelRoutingAtStart = MAL_TRUE;
    }


    // We always initialize every stage of the pipeline regardless of whether or not the stage is used because it simplifies
    // a few things when it comes to dynamically changing properties post-initialization.
    mal_result result = MAL_SUCCESS;

    // Pre format conversion.
    {
        mal_format_converter_config preFormatConverterConfig = mal_format_converter_config_init(
            pConfig->formatIn,
            mal_format_f32,
            pConfig->channelsIn,
            mal_dsp__pre_format_converter_on_read,
            pDSP
        );
        preFormatConverterConfig.ditherMode = pConfig->ditherMode;
        preFormatConverterConfig.noSSE2     = pConfig->noSSE2;
        preFormatConverterConfig.noAVX2     = pConfig->noAVX2;
        preFormatConverterConfig.noAVX512   = pConfig->noAVX512;
        preFormatConverterConfig.noNEON     = pConfig->noNEON;

        result = mal_format_converter_init(&preFormatConverterConfig, &pDSP->formatConverterIn);
        if (result != MAL_SUCCESS) {
            return result;
        }
    }

    // Post format conversion. The exact configuration for this depends on whether or not we are reading data directly from the client
    // or from an earlier stage in the pipeline.
    {
        mal_format_converter_config postFormatConverterConfig = mal_format_converter_config_init_new();
        postFormatConverterConfig.formatIn   = pConfig->formatIn;
        postFormatConverterConfig.formatOut  = pConfig->formatOut;
        postFormatConverterConfig.channels   = pConfig->channelsOut;
        postFormatConverterConfig.ditherMode = pConfig->ditherMode;
        postFormatConverterConfig.noSSE2     = pConfig->noSSE2;
        postFormatConverterConfig.noAVX2     = pConfig->noAVX2;
        postFormatConverterConfig.noAVX512   = pConfig->noAVX512;
        postFormatConverterConfig.noNEON     = pConfig->noNEON;
        if (pDSP->isPreFormatConversionRequired) {
            postFormatConverterConfig.onReadDeinterleaved = mal_dsp__post_format_converter_on_read_deinterleaved;
            postFormatConverterConfig.formatIn = mal_format_f32;
        } else {
            postFormatConverterConfig.onRead = mal_dsp__post_format_converter_on_read;
        }

        result = mal_format_converter_init(&postFormatConverterConfig, &pDSP->formatConverterOut);
        if (result != MAL_SUCCESS) {
            return result;
        }
    }

    // SRC
    {
        mal_src_config srcConfig = mal_src_config_init(
            pConfig->sampleRateIn,
            pConfig->sampleRateOut,
            ((pConfig->channelsIn < pConfig->channelsOut) ? pConfig->channelsIn : pConfig->channelsOut),
            mal_dsp__src_on_read_deinterleaved,
            pDSP
        );
        srcConfig.algorithm              = pConfig->srcAlgorithm;
        srcConfig.neverConsumeEndOfInput = pConfig->neverConsumeEndOfInput;
        srcConfig.noSSE2                 = pConfig->noSSE2;
        srcConfig.noAVX2                 = pConfig->noAVX2;
        srcConfig.noAVX512               = pConfig->noAVX512;
        srcConfig.noNEON                 = pConfig->noNEON;
        mal_copy_memory(&srcConfig.sinc, &pConfig->sinc, sizeof(pConfig->sinc));

        result = mal_src_init(&srcConfig, &pDSP->src);
        if (result != MAL_SUCCESS) {
            return result;
        }
    }

    // Channel conversion
    {
        mal_channel_router_config routerConfig = mal_channel_router_config_init(
            pConfig->channelsIn,
            pConfig->channelMapIn,
            pConfig->channelsOut,
            pConfig->channelMapOut,
            pConfig->channelMixMode,
            mal_dsp__channel_router_on_read_deinterleaved,
            pDSP);
        routerConfig.noSSE2   = pConfig->noSSE2;
        routerConfig.noAVX2   = pConfig->noAVX2;
        routerConfig.noAVX512 = pConfig->noAVX512;
        routerConfig.noNEON   = pConfig->noNEON;

        result = mal_channel_router_init(&routerConfig, &pDSP->channelRouter);
        if (result != MAL_SUCCESS) {
            return result;
        }
    }

    return MAL_SUCCESS;
}


mal_result mal_dsp_refresh_sample_rate(mal_dsp* pDSP)
{
    // The SRC stage will already have been initialized so we can just set it there.
    mal_src_set_input_sample_rate(&pDSP->src, pDSP->src.config.sampleRateIn);
    mal_src_set_output_sample_rate(&pDSP->src, pDSP->src.config.sampleRateOut);

    return MAL_SUCCESS;
}

mal_result mal_dsp_set_input_sample_rate(mal_dsp* pDSP, mal_uint32 sampleRateIn)
{
    if (pDSP == NULL) {
        return MAL_INVALID_ARGS;
    }

    // Must have a sample rate of > 0.
    if (sampleRateIn == 0) {
        return MAL_INVALID_ARGS;
    }

    // Must have been initialized with allowDynamicSampleRate.
    if (!pDSP->isDynamicSampleRateAllowed) {
        return MAL_INVALID_OPERATION;
    }

    mal_atomic_exchange_32(&pDSP->src.config.sampleRateIn, sampleRateIn);
    return mal_dsp_refresh_sample_rate(pDSP);
}

mal_result mal_dsp_set_output_sample_rate(mal_dsp* pDSP, mal_uint32 sampleRateOut)
{
    if (pDSP == NULL) {
        return MAL_INVALID_ARGS;
    }

    // Must have a sample rate of > 0.
    if (sampleRateOut == 0) {
        return MAL_INVALID_ARGS;
    }

    // Must have been initialized with allowDynamicSampleRate.
    if (!pDSP->isDynamicSampleRateAllowed) {
        return MAL_INVALID_OPERATION;
    }

    mal_atomic_exchange_32(&pDSP->src.config.sampleRateOut, sampleRateOut);
    return mal_dsp_refresh_sample_rate(pDSP);
}

mal_result mal_dsp_set_sample_rate(mal_dsp* pDSP, mal_uint32 sampleRateIn, mal_uint32 sampleRateOut)
{
    if (pDSP == NULL) {
        return MAL_INVALID_ARGS;
    }

    // Must have a sample rate of > 0.
    if (sampleRateIn == 0 || sampleRateOut == 0) {
        return MAL_INVALID_ARGS;
    }

    // Must have been initialized with allowDynamicSampleRate.
    if (!pDSP->isDynamicSampleRateAllowed) {
        return MAL_INVALID_OPERATION;
    }

    mal_atomic_exchange_32(&pDSP->src.config.sampleRateIn, sampleRateIn);
    mal_atomic_exchange_32(&pDSP->src.config.sampleRateOut, sampleRateOut);

    return mal_dsp_refresh_sample_rate(pDSP);
}

mal_uint64 mal_dsp_read(mal_dsp* pDSP, mal_uint64 frameCount, void* pFramesOut, void* pUserData)
{
    if (pDSP == NULL || pFramesOut == NULL) return 0;

    // Fast path.
    if (pDSP->isPassthrough) {
        if (frameCount <= 0xFFFFFFFF) {
            return (mal_uint32)pDSP->onRead(pDSP, (mal_uint32)frameCount, pFramesOut, pUserData);
        } else {
            mal_uint8* pNextFramesOut = (mal_uint8*)pFramesOut;

            mal_uint64 totalFramesRead = 0;
            while (totalFramesRead < frameCount) {
                mal_uint64 framesRemaining = (frameCount - totalFramesRead);
                mal_uint64 framesToReadRightNow = framesRemaining;
                if (framesToReadRightNow > 0xFFFFFFFF) {
                    framesToReadRightNow = 0xFFFFFFFF;
                }

                mal_uint32 framesRead = pDSP->onRead(pDSP, (mal_uint32)framesToReadRightNow, pNextFramesOut, pUserData);
                if (framesRead == 0) {
                    break;
                }

                pNextFramesOut  += framesRead * pDSP->channelRouter.config.channelsOut * mal_get_bytes_per_sample(pDSP->formatConverterOut.config.formatOut);
                totalFramesRead += framesRead;
            }

            return totalFramesRead;
        }
    }

    // Slower path. The real is done here. To do this all we need to do is read from the last stage in the pipeline.
    mal_assert(pDSP->isPostFormatConversionRequired == MAL_TRUE);

    mal_dsp_callback_data data;
    data.pDSP = pDSP;
    data.pUserDataForClient = pUserData;
    return mal_format_converter_read(&pDSP->formatConverterOut, frameCount, pFramesOut, &data);
}


typedef struct
{
    const void* pDataIn;
    mal_format formatIn;
    mal_uint32 channelsIn;
    mal_uint64 totalFrameCount;
    mal_uint64 iNextFrame;
    mal_bool32 isFeedingZeros;  // When set to true, feeds the DSP zero samples.
} mal_convert_frames__data;

mal_uint32 mal_convert_frames__on_read(mal_dsp* pDSP, mal_uint32 frameCount, void* pFramesOut, void* pUserData)
{
    (void)pDSP;

    mal_convert_frames__data* pData = (mal_convert_frames__data*)pUserData;
    mal_assert(pData != NULL);
    mal_assert(pData->totalFrameCount >= pData->iNextFrame);

    mal_uint32 framesToRead = frameCount;
    mal_uint64 framesRemaining = (pData->totalFrameCount - pData->iNextFrame);
    if (framesToRead > framesRemaining) {
        framesToRead = (mal_uint32)framesRemaining;
    }

    mal_uint32 frameSizeInBytes = mal_get_bytes_per_frame(pData->formatIn, pData->channelsIn);

    if (!pData->isFeedingZeros) {
        mal_copy_memory(pFramesOut, (const mal_uint8*)pData->pDataIn + (frameSizeInBytes * pData->iNextFrame), frameSizeInBytes * framesToRead);
    } else {
        mal_zero_memory(pFramesOut, frameSizeInBytes * framesToRead);
    }

    pData->iNextFrame += framesToRead;
    return framesToRead;
}

mal_dsp_config mal_dsp_config_init_new()
{
    mal_dsp_config config;
    mal_zero_object(&config);

    return config;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Miscellaneous Helpers
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void* mal_malloc(size_t sz)
{
    return MAL_MALLOC(sz);
}

void* mal_realloc(void* p, size_t sz)
{
    return MAL_REALLOC(p, sz);
}

void mal_free(void* p)
{
    MAL_FREE(p);
}

void* mal_aligned_malloc(size_t sz, size_t alignment)
{
    if (alignment == 0) {
        return 0;
    }

    size_t extraBytes = alignment-1 + sizeof(void*);

    void* pUnaligned = mal_malloc(sz + extraBytes);
    if (pUnaligned == NULL) {
        return NULL;
    }

    void* pAligned = (void*)(((mal_uintptr)pUnaligned + extraBytes) & ~((mal_uintptr)(alignment-1)));
    ((void**)pAligned)[-1] = pUnaligned;

    return pAligned;
}

void mal_aligned_free(void* p)
{
    mal_free(((void**)p)[-1]);
}

const char* mal_get_format_name(mal_format format)
{
    switch (format)
    {
        case mal_format_unknown: return "Unknown";
        case mal_format_u8:      return "8-bit Unsigned Integer";
        case mal_format_s16:     return "16-bit Signed Integer";
        case mal_format_s24:     return "24-bit Signed Integer (Tightly Packed)";
        case mal_format_s32:     return "32-bit Signed Integer";
        case mal_format_f32:     return "32-bit IEEE Floating Point";
        default:                 return "Invalid";
    }
}

void mal_blend_f32(float* pOut, float* pInA, float* pInB, float factor, mal_uint32 channels)
{
    for (mal_uint32 i = 0; i < channels; ++i) {
        pOut[i] = mal_mix_f32(pInA[i], pInB[i], factor);
    }
}


mal_uint32 mal_get_bytes_per_sample(mal_format format)
{
    mal_uint32 sizes[] = {
        0,  // unknown
        1,  // u8
        2,  // s16
        3,  // s24
        4,  // s32
        4,  // f32
    };
    return sizes[format];
}

#if defined(_MSC_VER)
    #pragma warning(pop)
#endif

#endif  // MINI_AL_IMPLEMENTATION


// REVISION HISTORY
// ================
//
// v0.8.13 - 2018-12-04
//   - Core Audio: Fix a bug with channel mapping.
//   - Fix a bug with channel routing where the back/left and back/right channels have the wrong weight.
//
// v0.8.12 - 2018-11-27
//   - Drop support for SDL 1.2. The Emscripten build now requires "-s USE_SDL=2".
//   - Fix a linking error with ALSA.
//   - Fix a bug on iOS where the device name is not set correctly.
//
// v0.8.11 - 2018-11-21
//   - iOS bug fixes.
//   - Minor tweaks to PulseAudio.
//
// v0.8.10 - 2018-10-21
//   - Core Audio: Fix a hang when uninitializing a device.
//   - Fix a bug where an incorrect value is returned from mal_device_stop().
//
// v0.8.9 - 2018-09-28
//   - Fix a bug with the SDL backend where device initialization fails.
//
// v0.8.8 - 2018-09-14
//   - Fix Linux build with the ALSA backend.
//   - Minor documentation fix.
//
// v0.8.7 - 2018-09-12
//   - Fix a bug with UWP detection.
//
// v0.8.6 - 2018-08-26
//   - Automatically switch the internal device when the default device is unplugged. Note that this is still in the
//     early stages and not all backends handle this the same way. As of this version, this will not detect a default
//     device switch when changed from the operating system's audio preferences (unless the backend itself handles
//     this automatically). This is not supported in exclusive mode.
//   - WASAPI and Core Audio: Add support for stream routing. When the application is using a default device and the
//     user switches the default device via the operating system's audio preferences, mini_al will automatically switch
//     the internal device to the new default. This is not supported in exclusive mode.
//   - WASAPI: Add support for hardware offloading via IAudioClient2. Only supported on Windows 8 and newer.
//   - WASAPI: Add support for low-latency shared mode via IAudioClient3. Only supported on Windows 10 and newer.
//   - Add support for compiling the UWP build as C.
//   - mal_device_set_recv_callback() and mal_device_set_send_callback() have been deprecated. You must now set this
//     when the device is initialized with mal_device_init*(). These will be removed in version 0.9.0.
//
// v0.8.5 - 2018-08-12
//   - Add support for specifying the size of a device's buffer in milliseconds. You can still set the buffer size in
//     frames if that suits you. When bufferSizeInFrames is 0, bufferSizeInMilliseconds will be used. If both are non-0
//     then bufferSizeInFrames will take priority. If both are set to 0 the default buffer size is used.
//   - Add support for the audio(4) backend to OpenBSD.
//   - Fix a bug with the ALSA backend that was causing problems on Raspberry Pi. This significantly improves the
//     Raspberry Pi experience.
//   - Fix a bug where an incorrect number of samples is returned from sinc resampling.
//   - Add support for setting the value to be passed to internal calls to CoInitializeEx().
//   - WASAPI and WinMM: Stop the device when it is unplugged.
//
// v0.8.4 - 2018-08-06
//   - Add sndio backend for OpenBSD.
//   - Add audio(4) backend for NetBSD.
//   - Drop support for the OSS backend on everything except FreeBSD and DragonFly BSD.
//   - Formats are now native-endian (were previously little-endian).
//   - Mark some APIs as deprecated:
//     - mal_src_set_input_sample_rate() and mal_src_set_output_sample_rate() are replaced with mal_src_set_sample_rate().
//     - mal_dsp_set_input_sample_rate() and mal_dsp_set_output_sample_rate() are replaced with mal_dsp_set_sample_rate().
//   - Fix a bug when capturing using the WASAPI backend.
//   - Fix some aliasing issues with resampling, specifically when increasing the sample rate.
//   - Fix warnings.
//
// v0.8.3 - 2018-07-15
//   - Fix a crackling bug when resampling in capture mode.
//   - Core Audio: Fix a bug where capture does not work.
//   - ALSA: Fix a bug where the worker thread can get stuck in an infinite loop.
//   - PulseAudio: Fix a bug where mal_context_init() succeeds when PulseAudio is unusable.
//   - JACK: Fix a bug where mal_context_init() succeeds when JACK is unusable.
//
// v0.8.2 - 2018-07-07
//   - Fix a bug on macOS with Core Audio where the internal callback is not called.
//
// v0.8.1 - 2018-07-06
//   - Fix compilation errors and warnings.
//
// v0.8 - 2018-07-05
//   - Changed MAL_IMPLEMENTATION to MINI_AL_IMPLEMENTATION for consistency with other libraries. The old
//     way is still supported for now, but you should update as it may be removed in the future.
//   - API CHANGE: Replace device enumeration APIs. mal_enumerate_devices() has been replaced with
//     mal_context_get_devices(). An additional low-level device enumration API has been introduced called
//     mal_context_enumerate_devices() which uses a callback to report devices.
//   - API CHANGE: Rename mal_get_sample_size_in_bytes() to mal_get_bytes_per_sample() and add
//     mal_get_bytes_per_frame().
//   - API CHANGE: Replace mal_device_config.preferExclusiveMode with mal_device_config.shareMode.
//     - This new config can be set to mal_share_mode_shared (default) or mal_share_mode_exclusive.
//   - API CHANGE: Remove excludeNullDevice from mal_context_config.alsa.
//   - API CHANGE: Rename MAL_MAX_SAMPLE_SIZE_IN_BYTES to MAL_MAX_PCM_SAMPLE_SIZE_IN_BYTES.
//   - API CHANGE: Change the default channel mapping to the standard Microsoft mapping.
//   - API CHANGE: Remove backend-specific result codes.
//   - API CHANGE: Changes to the format conversion APIs (mal_pcm_f32_to_s16(), etc.)
//   - Add support for Core Audio (Apple).
//   - Add support for PulseAudio.
//     - This is the highest priority backend on Linux (higher priority than ALSA) since it is commonly
//       installed by default on many of the popular distros and offer's more seamless integration on
//       platforms where PulseAudio is used. In addition, if PulseAudio is installed and running (which
//       is extremely common), it's better to just use PulseAudio directly rather than going through the
//       "pulse" ALSA plugin (which is what the "default" ALSA device is likely set to).
//   - Add support for JACK.
//   - Remove dependency on asound.h for the ALSA backend. This means the ALSA development packages are no
//     longer required to build mini_al.
//   - Remove dependency on dsound.h for the DirectSound backend. This fixes build issues with some
//     distributions of MinGW.
//   - Remove dependency on audioclient.h for the WASAPI backend. This fixes build issues with some
//     distributions of MinGW.
//   - Add support for dithering to format conversion.
//   - Add support for configuring the priority of the worker thread.
//   - Add a sine wave generator.
//   - Improve efficiency of sample rate conversion.
//   - Introduce the notion of standard channel maps. Use mal_get_standard_channel_map().
//   - Introduce the notion of default device configurations. A default config uses the same configuration
//     as the backend's internal device, and as such results in a pass-through data transmission pipeline.
//   - Add support for passing in NULL for the device config in mal_device_init(), which uses a default
//     config. This requires manually calling mal_device_set_send/recv_callback().
//   - Add support for decoding from raw PCM data (mal_decoder_init_raw(), etc.)
//   - Make mal_device_init_ex() more robust.
//   - Make some APIs more const-correct.
//   - Fix errors with SDL detection on Apple platforms.
//   - Fix errors with OpenAL detection.
//   - Fix some memory leaks.
//   - Fix a bug with opening decoders from memory.
//   - Early work on SSE2, AVX2 and NEON optimizations.
//   - Miscellaneous bug fixes.
//   - Documentation updates.
//
// v0.7 - 2018-02-25
//   - API CHANGE: Change mal_src_read_frames() and mal_dsp_read_frames() to use 64-bit sample counts.
//   - Add decoder APIs for loading WAV, FLAC, Vorbis and MP3 files.
//   - Allow opening of devices without a context.
//     - In this case the context is created and managed internally by the device.
//   - Change the default channel mapping to the same as that used by FLAC.
//   - Fix build errors with macOS.
//
// v0.6c - 2018-02-12
//   - Fix build errors with BSD/OSS.
//
// v0.6b - 2018-02-03
//   - Fix some warnings when compiling with Visual C++.
//
// v0.6a - 2018-01-26
//   - Fix errors with channel mixing when increasing the channel count.
//   - Improvements to the build system for the OpenAL backend.
//   - Documentation fixes.
//
// v0.6 - 2017-12-08
//   - API CHANGE: Expose and improve mutex APIs. If you were using the mutex APIs before this version you'll
//     need to update.
//   - API CHANGE: SRC and DSP callbacks now take a pointer to a mal_src and mal_dsp object respectively.
//   - API CHANGE: Improvements to event and thread APIs. These changes make these APIs more consistent.
//   - Add support for SDL and Emscripten.
//   - Simplify the build system further for when development packages for various backends are not installed.
//     With this change, when the compiler supports __has_include, backends without the relevant development
//     packages installed will be ignored. This fixes the build for old versions of MinGW.
//   - Fixes to the Android build.
//   - Add mal_convert_frames(). This is a high-level helper API for performing a one-time, bulk conversion of
//     audio data to a different format.
//   - Improvements to f32 -> u8/s16/s24/s32 conversion routines.
//   - Fix a bug where the wrong value is returned from mal_device_start() for the OpenSL backend.
//   - Fixes and improvements for Raspberry Pi.
//   - Warning fixes.
//
// v0.5 - 2017-11-11
//   - API CHANGE: The mal_context_init() function now takes a pointer to a mal_context_config object for
//     configuring the context. The works in the same kind of way as the device config. The rationale for this
//     change is to give applications better control over context-level properties, add support for backend-
//     specific configurations, and support extensibility without breaking the API.
//   - API CHANGE: The alsa.preferPlugHW device config variable has been removed since it's not really useful for
//     anything anymore.
//   - ALSA: By default, device enumeration will now only enumerate over unique card/device pairs. Applications
//     can enable verbose device enumeration by setting the alsa.useVerboseDeviceEnumeration context config
//     variable.
//   - ALSA: When opening a device in shared mode (the default), the dmix/dsnoop plugin will be prioritized. If
//     this fails it will fall back to the hw plugin. With this change the preferExclusiveMode config is now
//     honored. Note that this does not happen when alsa.useVerboseDeviceEnumeration is set to true (see above)
//     which is by design.
//   - ALSA: Add support for excluding the "null" device using the alsa.excludeNullDevice context config variable.
//   - ALSA: Fix a bug with channel mapping which causes an assertion to fail.
//   - Fix errors with enumeration when pInfo is set to NULL.
//   - OSS: Fix a bug when starting a device when the client sends 0 samples for the initial buffer fill.
//
// v0.4 - 2017-11-05
//   - API CHANGE: The log callback is now per-context rather than per-device and as is thus now passed to
//     mal_context_init(). The rationale for this change is that it allows applications to capture diagnostic
//     messages at the context level. Previously this was only available at the device level.
//   - API CHANGE: The device config passed to mal_device_init() is now const.
//   - Added support for OSS which enables support on BSD platforms.
//   - Added support for WinMM (waveOut/waveIn).
//   - Added support for UWP (Universal Windows Platform) applications. Currently C++ only.
//   - Added support for exclusive mode for selected backends. Currently supported on WASAPI.
//   - POSIX builds no longer require explicit linking to libpthread (-lpthread).
//   - ALSA: Explicit linking to libasound (-lasound) is no longer required.
//   - ALSA: Latency improvements.
//   - ALSA: Use MMAP mode where available. This can be disabled with the alsa.noMMap config.
//   - ALSA: Use "hw" devices instead of "plughw" devices by default. This can be disabled with the
//     alsa.preferPlugHW config.
//   - WASAPI is now the highest priority backend on Windows platforms.
//   - Fixed an error with sample rate conversion which was causing crackling when capturing.
//   - Improved error handling.
//   - Improved compiler support.
//   - Miscellaneous bug fixes.
//
// v0.3 - 2017-06-19
//   - API CHANGE: Introduced the notion of a context. The context is the highest level object and is required for
//     enumerating and creating devices. Now, applications must first create a context, and then use that to
//     enumerate and create devices. The reason for this change is to ensure device enumeration and creation is
//     tied to the same backend. In addition, some backends are better suited to this design.
//   - API CHANGE: Removed the rewinding APIs because they're too inconsistent across the different backends, hard
//     to test and maintain, and just generally unreliable.
//   - Added helper APIs for initializing mal_device_config objects.
//   - Null Backend: Fixed a crash when recording.
//   - Fixed build for UWP.
//   - Added support for f32 formats to the OpenSL|ES backend.
//   - Added initial implementation of the WASAPI backend.
//   - Added initial implementation of the OpenAL backend.
//   - Added support for low quality linear sample rate conversion.
//   - Added early support for basic channel mapping.
//
// v0.2 - 2016-10-28
//   - API CHANGE: Add user data pointer as the last parameter to mal_device_init(). The rationale for this
//     change is to ensure the logging callback has access to the user data during initialization.
//   - API CHANGE: Have device configuration properties be passed to mal_device_init() via a structure. Rationale:
//     1) The number of parameters is just getting too much.
//     2) It makes it a bit easier to add new configuration properties in the future. In particular, there's a
//        chance there will be support added for backend-specific properties.
//   - Dropped support for f64, A-law and Mu-law formats since they just aren't common enough to justify the
//     added maintenance cost.
//   - DirectSound: Increased the default buffer size for capture devices.
//   - Added initial implementation of the OpenSL|ES backend.
//
// v0.1 - 2016-10-21
//   - Initial versioned release.


/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/
