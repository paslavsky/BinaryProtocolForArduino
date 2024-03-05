#ifndef BPA_ERRORS_H
#define BPA_ERRORS_H

namespace bpa {

enum ErrorCode {
    NO_ERROR = 0,
    DEVICE_NOT_CONNECTED = 1,
    DEVICE_LOST = 2,
    INCORRECT_FORMAT_ERROR = 3,
    
};

} // namespace bpa

#endif // BPA_ERRORS_H
