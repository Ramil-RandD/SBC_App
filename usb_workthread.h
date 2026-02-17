#ifndef USB_WORKTHREAD_H
#define USB_WORKTHREAD_H

#include <QMutex>
#include <QThread>
#include <QWaitCondition>
#include <QElapsedTimer>
#include <libusb-1.0/libusb.h>
//#include <libusb-MinGW-Win32\include\libusb-1.0\libusb.h>
#include <ringbuffer.h>
#include <usb_global.h>

class UsbWorkThread : public QThread
{
    Q_OBJECT

signals:
    void consolePutData(const QString &data, quint8 priority);
    void consoleAdcFile(const quint8 *adc_data, quint32 size);
    void postTxDataToSerialPort(const uint8_t *p_data, int len);
    void postWaitToSerialPort();
    void postDataToStm32H7(const uint8_t *p_data, const int length);
    void usbInitTimeoutStart(int timeout_ms);
    void hsDataReceived();
    void srpModeSet(uint8_t mode);

public slots:
    void sendHsCommand(uint8_t Command, uint32_t Length, const uint8_t *p_Data);

public:
    explicit UsbWorkThread(QObject *parent = nullptr);
    ~UsbWorkThread();

    void USB_ThreadStart();
    void USB_Init();
    void USB_Deinit();
    void USB_StopTransmit();

    // Init 'states'
    typedef enum {
        INIT = 0, OPEN_DEV, DEV_TAKE_CONTROL, CLAIM_INTERFACE, TRANSMIT_RECEIVE_INIT, INIT_END
    } UsbInitState;

    UsbInitState InitState = UsbInitState::INIT;

    uint8_t UserTxBuffer[65536];

    bool start_transmit = false;
    bool start_receive = false;

    int UserRxBuffer_len = 0;
    int UserTxBuffer_len = 0;
    int pStartData = 0;

    bool usb_receiver_drop = false;     // Drop any received usb data when this flag is active

    const int timeoutUsbInit_ms = 1000;

    uint32_t stm32_ready_data_size = 0;

    bool mod_tx_rx_sequence = false;
    bool agc_is_active = false;

private:
    void run() override;

    bool usbInitialization();
    void USB_Reconnect();
    void USB_ReceiveTransmitInit();
    void USB_StartReceive(uint8_t *p_rx_buffer_offset);
    void USB_StartTransmit();
    void USB_StopReceive();
    void parseHsData();
    void parseInternalCommData();
    void Default_HsDataParser(uint8_t *p_data);

    // Hack to use libusb's 'C'-callback functions in 'C++' project
    // Use 'user_data' to pass pointer to 'this'
    static void LIBUSB_CALL static_tx_callback(struct libusb_transfer *transfer)
    {
        return reinterpret_cast<UsbWorkThread*>(transfer->user_data)->tx_callback(transfer);
    }
    static void LIBUSB_CALL static_rx_callback(struct libusb_transfer *transfer)
    {
        return reinterpret_cast<UsbWorkThread*>(transfer->user_data)->rx_callback(transfer);
    }
    static int LIBUSB_CALL static_hotplug_callback(libusb_context *ctx, libusb_device *device, libusb_hotplug_event event, void *user_data)
    {
        return reinterpret_cast<UsbWorkThread*>(user_data)->hotplug_callback(ctx, device, event, user_data);
    }

    void LIBUSB_CALL tx_callback(struct libusb_transfer *transfer);
    void LIBUSB_CALL rx_callback(struct libusb_transfer *transfer);
    int LIBUSB_CALL hotplug_callback(libusb_context *ctx, libusb_device *device, libusb_hotplug_event event, void *user_data);


    libusb_context *context = nullptr;
    libusb_device_handle *handle = nullptr;

    struct libusb_transfer *t_rx = nullptr;
    struct libusb_transfer *t_tx = nullptr;

    bool m_quit = false;
    bool receive_in_progress = false;
    bool transmit_in_progress = false;

    int rx_complete_flag = 0;
    int tx_complete_flag = 0;

    QElapsedTimer rx_timeout_timer;
    QElapsedTimer console_spam_timer;
    QElapsedTimer synchro_measure_timer;

    const qint64 rx_timeout_ms = 50;//10;            // usb rx timeout between transfers of one big packet, ms

    libusb_hotplug_callback_handle h_hotplug;   // libusb callback handle for hotplug event

    typedef void(*DataParserCallbackType)(uint8_t *p_data);

    DataParserCallbackType m_callback = NULL;
};

#endif // USB_WORKTHREAD_H
