juce_add_plugin(CK_Panning
    COMPANY_NAME "YourCompany"
    PLUGIN_MANUFACTURER_CODE Juce
    PLUGIN_CODE CKPN
    FORMATS VST3 AU AAX
    PRODUCT_NAME "CK Panning"
    
    # Source files
    SOURCES
        PluginProcessor.cpp
        PluginProcessor.h
        PluginEditor.cpp
        PluginEditor.h
)
