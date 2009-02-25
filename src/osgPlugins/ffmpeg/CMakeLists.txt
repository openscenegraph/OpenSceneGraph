INCLUDE_DIRECTORIES( ${FFMPEG_INCLUDE_DIRS} )

SET(TARGET_SRC 
    FFmpegClocks.cpp
    FFmpegDecoderAudio.cpp
    FFmpegDecoder.cpp
    FFmpegDecoderVideo.cpp
    FFmpegImageStream.cpp
    ReaderWriterFFmpeg.cpp
)

SET(TARGET_H
    AudioSinkInterface.hpp
    BoundedMessageQueue.hpp
    FFmpegClocks.hpp
    FFmpegDecoderAudio.hpp
    FFmpegDecoder.hpp
    FFmpegDecoderVideo.hpp
    FFmpegHeaders.hpp
    FFmpegPacket.hpp
    MessageQueue.hpp
)

SET(TARGET_EXTERNAL_LIBRARIES ${FFMPEG_LIBRARIES} )


#### end var setup  ###
SETUP_PLUGIN(ffmpeg)
