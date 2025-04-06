# Plugin #######################################################################

# juce_set_vst2_sdk_path(...)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

juce_add_plugin(${PREDEF_PROJECT_NAME}
    PRODUCT_NAME "${PREDEF_PROJECT_NAME}"
    COMPANY_NAME "Csaba Koza"
    FORMATS 
        Standalone
        VST3
    PLUGIN_NAME "${PREDEF_PROJECT_NAME}"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    VST3_CATEGORIES
        Fx
    BUNDLE_ID "com.Csaba_Koza.${PREDEF_PROJECT_NAME}"
)

target_compile_features(Patch PUBLIC cxx_std_20)

target_include_directories(Patch PUBLIC
                           ${CMAKE_SOURCE_DIR}/source)

target_sources(Patch PUBLIC
    PluginProcessor.cpp
    PluginEditor.cpp

    Core.cpp
    Instance.cpp
    )

target_compile_definitions(Patch PUBLIC
    JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
    JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:${PREDEF_PROJECT_NAME},JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:${PREDEF_PROJECT_NAME},JUCE_VERSION>"
)

# juce_add_binary_data(GuiAppData SOURCES ...)

target_link_libraries(Patch PUBLIC
    # GuiAppData
    juce::juce_core
    # juce::juce_data_structures
    juce::juce_events
    juce::juce_graphics
    juce::juce_audio_basics
    # juce::juce_audio_formats
    juce::juce_gui_basics
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
    juce::juce_recommended_lto_flags
)
