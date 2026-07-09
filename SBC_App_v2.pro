QT += widgets serialport dbus
requires(qtConfig(combobox))

TARGET = SBC_App_v2
TEMPLATE = app

# Version, max 255.255.255
VERSION = 1.1.5.1

DEFINES += VERSION_STRING=\\\"$${VERSION}\\\"

SOURCES += \
    atomic_vars.cpp \
    blackbox.cpp \
    crc16.cpp \
    crc8.cpp \
    gpio_tracker.cpp \
    indigo_base_protocol.cpp \
    main.cpp \
    mainwindow.cpp \
    median_window_filter.cpp \
    message_box.cpp \
    mod_transmitter_thread.cpp \
    qam_decoder/HS_EWL_LineEqualizer.cpp \
    qam_decoder/HS_EWL_LineEqualizer_data.cpp \
    qam_decoder/HS_EWL_LineEqualizer_initialize.cpp \
    qam_decoder/HS_EWL_LineEqualizer_terminate.cpp \
    qam_decoder/eml_fftshift.cpp \
    qam_decoder/fft.cpp \
    qam_decoder/FFTImplementationCallback.cpp \
    qam_decoder/fftshift.cpp \
    qam_decoder/FIRDecimator.cpp \
    qam_decoder/getGrayMapping.cpp \
    qam_decoder/HS_EWL_DEMOD_QAM.cpp \
    qam_decoder/HS_EWL_DEMOD_QAM_data.cpp \
    qam_decoder/HS_EWL_DEMOD_QAM_data.h \
    qam_decoder/HS_EWL_DEMOD_QAM_initialize.cpp \
    qam_decoder/HS_EWL_DEMOD_QAM_rtwutil.cpp \
    qam_decoder/HS_EWL_DEMOD_QAM_terminate.cpp \
    qam_decoder/HS_EWL_FREQ_ACQ.cpp \
    qam_decoder/HS_EWL_FREQ_EST_FOR_SWEEP.cpp \
    qam_decoder/HS_EWL_TR_FUN_EST.cpp \
    qam_decoder/ifft.cpp \
    qam_decoder/ifftshift.cpp \
    qam_decoder/interp1.cpp \
    qam_decoder/qam_init.cpp \
    qam_decoder/minOrMax.cpp \
    qam_decoder/movSumProdOrMean.cpp \
    qam_decoder/qammod.cpp \
    qam_decoder/RaisedCosineReceiveFilter.cpp \
    qam_decoder/rat.cpp \
    qam_decoder/rtGetInf.cpp \
    qam_decoder/rtGetNaN.cpp \
    qam_decoder/rt_nonfinite.cpp \
    qam_decoder/setup.cpp \
    qam_decoder/xcorr.cpp \
    qam_thread.cpp \
    ringbuffer.cpp \
    rs_decoder.cpp \
    console.cpp \
    sin_freq_sweep_thread.cpp \
    statistics.cpp \
    synchro_watcher.cpp \
    usb_workthread.cpp \
    IndigoProtocol/SharedData/sample_flags_handler.cpp \
    IndigoProtocol/V1/CRC8.cpp \
    IndigoProtocol/V1/CRC16.cpp \
    IndigoProtocol/V1/IndigoBaseProtocolBaseParserV1.cpp \
    IndigoProtocol/V1/IndigoBaseProtocolBuilderV1.cpp \
    IndigoProtocol/V1/IndigoBaseProtocolFrameBuilderV1.cpp \
    IndigoProtocol/V1/IndigoBaseProtocolFrameParserV1.cpp \
    IndigoProtocol/V1/IndigoBaseProtocolParserV1.cpp \
    IndigoProtocol/V1/IndigoBaseProtocolUsbBuilderV1.cpp \
    IndigoProtocol/V1/IndigoProtocolCommandBuilder.cpp \
    IndigoProtocol/V1/IndigoProtocolHelpClasses.cpp

CONFIG += c++11

#-------------------------------------------------
# Add LibUSB library
#-------------------------------------------------
win32 {
    LIBS += -L./ -llibusb-1.0
}
unix {
    LIBS += -L./ -lusb-1.0
}

#-------------------------------------------------
# Make sure to add the required libraries or
# frameoworks for the demo work depending on
# what OS is being used
#-------------------------------------------------
macx: LIBS += -framework CoreFoundation
win32: LIBS += -lSetupAPI

#-------------------------------------------------
# Make sure output directory for object file and
# executable is in the correct subdirectory
#-------------------------------------------------
macx {
    DESTDIR = mac
    OBJECTS_DIR = mac
    MOC_DIR = mac
    UI_DIR = mac
    RCC_DIR = mac
}
unix: !macx {
    DESTDIR = linux
    OBJECTS_DIR = linux
    MOC_DIR = linux
    UI_DIR = linux
    RCC_DIR = linux
}
win32 {
    DESTDIR = windows
    OBJECTS_DIR = windows
    MOC_DIR = windows
    UI_DIR = windows
    RCC_DIR = windows
}

OTHER_FILES +=

win32: LIBS += -L$$PWD/ -llibusb-1.0

INCLUDEPATH += $$PWD/
DEPENDPATH += $$PWD/

HEADERS += \
    ../SRP_HS_USB_PROTOCOL/SRP_HS_USB_Protocol.h \
    IndigoProtocol/constants.h \
    agc_algorithm.h \
    atomic_vars.h \
    blackbox.h \
    crc16.h \
    crc8.h \
    gpio_tracker.h \
    indigo_base_protocol.h \
    mainwindow.h \
    median_window_filter.h \
    message_box.h \
    mod_transmitter_thread.h \
    qam_common.h \
    qam_decoder/eml_fftshift.h \
    qam_decoder/fft.h \
    qam_decoder/FFTImplementationCallback.h \
    qam_decoder/fftshift.h \
    qam_decoder/FIRDecimator.h \
    qam_decoder/getGrayMapping.h \
    qam_decoder/HS_EWL_DEMOD_QAM.h \
    qam_decoder/HS_EWL_DEMOD_QAM_initialize.h \
    qam_decoder/HS_EWL_DEMOD_QAM_rtwutil.h \
    qam_decoder/HS_EWL_DEMOD_QAM_terminate.h \
    qam_decoder/HS_EWL_DEMOD_QAM_types.h \
    qam_decoder/HS_EWL_FREQ_ACQ.h \
    qam_decoder/HS_EWL_FREQ_EST_FOR_SWEEP.h \
    qam_decoder/HS_EWL_TR_FUN_EST.h \
    qam_decoder/ifft.h \
    qam_decoder/ifftshift.h \
    qam_decoder/qam_init.h \
    qam_decoder/math_sweep.h \
    qam_decoder/interp1.h \
    qam_decoder/minOrMax.h \
    qam_decoder/qammod.h \
    qam_decoder/movSumProdOrMean.h \
    qam_decoder/qam_decoder.h \
    qam_decoder/RaisedCosineReceiveFilter.h \
    qam_decoder/rat.h \
    qam_decoder/rt_defines.h \
    qam_decoder/rtGetInf.h \
    qam_decoder/rtGetNaN.h \
    qam_decoder/rt_nonfinite.h \
    qam_decoder/rtwtypes.h \
    qam_decoder/setup.h \
    qam_decoder/signal.h \
    qam_decoder/signal_sweep.h \
    qam_decoder/tmwtypes.h \
    qam_decoder/xcorr.h \
    qam_thread.h \
    ringbuffer.h \
    rs_decoder.h \
    settingsdialog.h \
    console.h \
    sin_freq_sweep_thread.h \
    srp_mod_protocol.h \
    statistics.h \
    synchro_watcher.h \
    usb_global.h \
    usb_workthread.h \
    IndigoProtocol/IndigoBaseTypes.h \
    IndigoProtocol/SharedData/mem_packet_sh.h \
    IndigoProtocol/SharedData/sample_flags_handler.h \
    IndigoProtocol/SharedData/SensorBase.h \
    IndigoProtocol/SharedData/SensorData.h \
    IndigoProtocol/V1/CommandBodyHeader.h \
    IndigoProtocol/V1/CommandHeaderHeader.h \
    IndigoProtocol/V1/CRC8.h \
    IndigoProtocol/V1/CRC16.h \
    IndigoProtocol/V1/IndigoBaseProtocolBaseParserV1.h \
    IndigoProtocol/V1/IndigoBaseProtocolBuilderV1.h \
    IndigoProtocol/V1/IndigoBaseProtocolFrameBuilderV1.h \
    IndigoProtocol/V1/IndigoBaseProtocolFrameParserV1.h \
    IndigoProtocol/V1/IndigoBaseProtocolParserV1.h \
    IndigoProtocol/V1/IndigoBaseProtocolUsbBuilderV1.h \
    IndigoProtocol/V1/IndigoBaseTypesV1.h \
    IndigoProtocol/V1/IndigoProtocolCommandBuilder.h \
    IndigoProtocol/V1/IndigoProtocolHelpClasses.h \
    IndigoProtocol/V1/TransportProtocolHeader.h

FORMS += \
    mainwindow.ui

RESOURCES += \
    SBC_App_v2.qrc

unix {
    INCLUDEPATH += /usr/lib/gcc/x86_64-linux-gnu/9/include
}

SUBDIRS += \
    qam_decoder/HS_EWL_tester_6.pro

DISTFILES += \
    qam_decoder/rtw_proj.tmw
