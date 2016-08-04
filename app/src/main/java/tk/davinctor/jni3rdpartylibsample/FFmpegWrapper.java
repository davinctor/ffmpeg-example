package tk.davinctor.jni3rdpartylibsample;

import android.support.annotation.IntDef;
import android.support.annotation.NonNull;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;

/**
 * 03.08.16
 */
public class FFmpegWrapper {

    static {
        System.loadLibrary("jni_lib");
    }

    @ResultCode
    public static native int init(@NonNull String fileName);

    public static native int finish();

    public static native int[] getVideoResolution();

    public static native double getVideoDuration();

    public static native double getVideoFrameRate();

    public static native int prepareDisplay(Object bitmap, int width, int height);

    public static native int getVideoFrame();

    public static String getRationaleMessageByResultCode(@ResultCode int resultCode) {
        return ResultCodes.valueOfResultCode(resultCode);
    }

    @SuppressWarnings("WeakerAccess")
    @IntDef
    @Retention(RetentionPolicy.SOURCE)
    public @interface ResultCode {
        int SUCCESS = 0;
        int ERROR_ALLOCATE_MEMORY = -1;
        int ERROR_OPEN_FILE = -2;
        int ERROR_FIND_STREAM_INFO = -3;
        int ERROR_VIDEO_STREAM_NOT_FOUND = -4;
        int ERROR_CODEC_NOT_FOUND = -5;
        int ERROR_OPEN_CODEC = -6;
        int ERROR_UNKNOWN = -1488;
    }

    public enum ResultCodes {
        SUCCESS(ResultCode.SUCCESS, "Success"),
        ERROR_ALLOCATE_MEMORY(ResultCode.ERROR_ALLOCATE_MEMORY, "Couldn't allocate memory"),
        ERROR_OPEN_FILE(ResultCode.ERROR_OPEN_FILE, "Couldn't open file"),
        ERROR_FIND_STREAM_INFO(ResultCode.ERROR_FIND_STREAM_INFO, "Couldn't find stream info"),
        ERROR_VIDEO_STREAM_NOT_FOUND(ResultCode.ERROR_VIDEO_STREAM_NOT_FOUND, "Couldn't find video stream"),
        ERROR_CODEC_NOT_FOUND(ResultCode.ERROR_CODEC_NOT_FOUND, "Couldn't find codec"),
        ERROR_OPEN_CODEC(ResultCode.ERROR_OPEN_CODEC, "Couldn't open codec"),
        ERROR_UNKNOWN(ResultCode.ERROR_UNKNOWN, "Unknown error");

        private int errorCode;
        private String rationaleMessage;

        ResultCodes(@ResultCode int errorCode, @NonNull String rationaleMessage) {
            this.errorCode = errorCode;
            this.rationaleMessage = rationaleMessage;
        }

        @ResultCode
        public int getErrorCode() {
            return errorCode;
        }

        public String getRationaleMessage() {
            return rationaleMessage;
        }

        public static String valueOfResultCode(@ResultCode int resultCode) {
            switch (resultCode) {
                case ResultCode.SUCCESS:
                    return SUCCESS.rationaleMessage;
                case ResultCode.ERROR_ALLOCATE_MEMORY:
                    return ERROR_ALLOCATE_MEMORY.rationaleMessage;
                case ResultCode.ERROR_OPEN_FILE:
                    return ERROR_OPEN_FILE.rationaleMessage;
                case ResultCode.ERROR_FIND_STREAM_INFO:
                    return ERROR_FIND_STREAM_INFO.rationaleMessage;
                case ResultCode.ERROR_VIDEO_STREAM_NOT_FOUND:
                    return ERROR_VIDEO_STREAM_NOT_FOUND.rationaleMessage;
                case ResultCode.ERROR_CODEC_NOT_FOUND:
                    return ERROR_CODEC_NOT_FOUND.rationaleMessage;
                case ResultCode.ERROR_OPEN_CODEC:
                    return ERROR_OPEN_CODEC.rationaleMessage;
                default:
                    return ERROR_UNKNOWN.rationaleMessage;
            }
        }
    }
}
