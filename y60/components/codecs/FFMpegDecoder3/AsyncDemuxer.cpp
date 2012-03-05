#include "AsyncDemuxer.h"

#include <asl/base/Logger.h>
#include <asl/base/string_functions.h>

#define DB(x) x

namespace {
    const unsigned int PACKET_QUEUE_SIZE = 500;
}

namespace y60 {

AsyncDemuxer::AsyncDemuxer(AVFormatContext * myFormatContext)
    : _myFormatContext(myFormatContext), _isSeeking(false), _isEOF(false) {
}

AsyncDemuxer::~AsyncDemuxer() {
    stop();
}

void
AsyncDemuxer::join() {
    AC_DEBUG << "AsyncDemuxer::join";
    _myDemuxThread->interrupt();
    _myDemuxThread->join();
}

void
AsyncDemuxer::stop() {
    AC_DEBUG << "AsyncDemuxer::stop";
    _myDemuxThread->interrupt();
    _myDemuxThread->join();
    clearPacketCache();
}
void
AsyncDemuxer::start() {
    AC_DEBUG << "AsyncDemuxer::start forking new thread";
    if (_myDemuxThread && !_myDemuxThread->timed_join(boost::posix_time::millisec(1))) {
        AC_WARNING<<"demux thread already running";
        _myDemuxThread->interrupt();
        _myDemuxThread->join();
    }
    _myDemuxThread = DemuxThreadPtr(new boost::thread( boost::bind( &AsyncDemuxer::run, this)));
}

void
AsyncDemuxer::run() {
    try {
        while(true) {
            DB(AC_TRACE << "---AsyncDemuxer::run: ");
            boost::this_thread::interruption_point();
            {
                boost::mutex::scoped_lock lock(_myMutex);
                while(_isSeeking) {
                    _myCondition.wait(lock);
                }
            }
            //XXX: rethink that: add condition: wait as long as queues are full
            if (queuesFull()) {
                boost::this_thread::sleep(boost::posix_time::millisec(10));
                //boost::this_thread::yield();
                continue;
            }
            AVPacket * myPacket = new AVPacket;
            memset(myPacket, 0, sizeof(AVPacket));

            int ret = av_read_frame(_myFormatContext, myPacket);
            DB(AC_TRACE << "---AsyncDemuxer::av_read_frame ret: "<<ret);
            if (ret < 0) {
                AC_DEBUG << "---AsyncDemuxer::run: EOF end of file. stream_index: "<<myPacket->stream_index;
                std::map<int, PacketQueuePtr>::iterator it = _myPacketQueues.find(myPacket->stream_index);
                if ( it != _myPacketQueues.end()) {
                    PacketMsgPtr p = PacketMsgPtr(new PacketMsg());
                    it->second->push(p);
                }
                av_free_packet(myPacket);
                delete myPacket;
                _isEOF = true;
                boost::mutex::scoped_lock lock(_myMutex);
                _myCondition.wait(lock);
            } else {
                putPacket(myPacket);
            }
        }
    } catch (boost::thread_interrupted &) {
        AC_DEBUG << "---AsyncDemuxer::run thread_interrupted";
    }
}

void
AsyncDemuxer::putPacket(AVPacket * thePacket) {
    std::map<int, PacketQueuePtr>::iterator it = _myPacketQueues.find(thePacket->stream_index);
    if ( it != _myPacketQueues.end()) {
        DB(AC_TRACE << "---AsyncDemuxer::putPacket, caching packet for stream: "<<it->first<<" queue size: " <<it->second->size()<<" , specs dts: "<<thePacket->dts << " pts: "<<thePacket->pts);
        // Without av_dup_packet, ffmpeg reuses myPacket->data at first
        // opportunity and trashes our memory.
        if (thePacket) {
            av_dup_packet(thePacket);
        }
        PacketQueue & myQueue = *(it->second);
        PacketMsgPtr p = PacketMsgPtr(new PacketMsg(thePacket));
        myQueue.push(p);
    } else if (thePacket) {
        DB(AC_DEBUG << "---AsyncDemuxer::putPacket rejecting packet for stream: "<<thePacket->stream_index);
        av_free_packet(thePacket);
        delete thePacket;
        thePacket = 0;
    }
}

AVPacket*
AsyncDemuxer::getPacket(const int theStreamIndex) {
    AC_TRACE<<"AsyncDemuxer::getPacket for stream: "<<theStreamIndex;
    std::map<int, PacketQueuePtr>::iterator it = _myPacketQueues.find(theStreamIndex);
    if (it == _myPacketQueues.end()) {
        AC_ERROR << "AsyncDemuxer::getPacket called with nonexistent stream index " << theStreamIndex << ".";
    }
    PacketMsgPtr p;
    //XXX: this triggers sometimes and should never -> investigate
    if(!it->second->timed_wait_and_pop<boost::posix_time::millisec>(p, boost::posix_time::millisec(5000))) {
        stop();
        throw asl::Exception("demuxer can't deliver anymore packets", PLUS_FILE_LINE);
    }
    return p->getPacket();
}

void
AsyncDemuxer::enableStream(const int theStreamIndex) {
    AC_INFO<<"AsyncDemuxer::enableStream with index: "<<theStreamIndex;
    _myPacketQueues[theStreamIndex] = PacketQueuePtr(new PacketQueue());
}

void
AsyncDemuxer::clearPacketCache() {
    AC_DEBUG<<"AsyncDemuxer::clearPacketCache";
    for (std::map<int, PacketQueuePtr>::iterator it = _myPacketQueues.begin(); it != _myPacketQueues.end(); ++it) {
        PacketMsgPtr p;
        while(it->second->try_pop(p)) {
            p->freePacket();
        }
    }
}

void
AsyncDemuxer::clearPacketCache(const int theStreamIndex) {
    AC_DEBUG<<"AsyncDemuxer::clearPacketCache for stream: "<<theStreamIndex;
    std::map<int, PacketQueuePtr>::iterator it = _myPacketQueues.find(theStreamIndex);
    if (it == _myPacketQueues.end()) {
        AC_ERROR << "AsyncDemuxer::clearPacketCache called with nonexistent stream index " << theStreamIndex;
    }
    PacketMsgPtr p;
    while(it->second->try_pop(p)) {
        p->freePacket();
    }
}

void
AsyncDemuxer::seek(double theDestTime) {
    AC_DEBUG<<"AsyncDemuxer::seek to "<<theDestTime;

    for (std::map<int, PacketQueuePtr>::iterator it = _myPacketQueues.begin(); it != _myPacketQueues.end(); ++it) {
        seek(theDestTime, it->first);
    }
}

void
AsyncDemuxer::seek(double theDestTime, const int theStreamIndex) {
    AC_DEBUG<<"AsyncDemuxer::seek stream: "<<theStreamIndex<<" to destTime: "<<theDestTime;
    std::map<int, PacketQueuePtr>::iterator it = _myPacketQueues.find(theStreamIndex);
    if (it == _myPacketQueues.end()) {
        throw asl::Exception("AsyncDemuxer::seek called with nonexistent stream index " + asl::as_string(theStreamIndex), PLUS_FILE_LINE);
    }
    boost::mutex::scoped_lock lock(_myMutex);
    _isSeeking = true;
    clearPacketCache(it->first);
    int64_t mySeekTimeInTimeBaseUnits = int64_t(theDestTime / av_q2d(_myFormatContext->streams[theStreamIndex]->time_base));
    /*int myResult =*/ av_seek_frame(_myFormatContext, theStreamIndex,
                            mySeekTimeInTimeBaseUnits, AVSEEK_FLAG_BACKWARD);
    avcodec_flush_buffers(_myFormatContext->streams[theStreamIndex]->codec);
    if (_isEOF) {
        _isEOF = false;
    }
    _isSeeking = false;
    AC_DEBUG<<"AsyncDemuxer::Seek DONE";
    _myCondition.notify_one();
}

void
AsyncDemuxer::dump() const {
    AC_DEBUG << "Demux list sizes: ";
    for (std::map<int, PacketQueuePtr>::const_iterator it = _myPacketQueues.begin(); it != _myPacketQueues.end(); ++it) {
        AC_DEBUG << "  " << it->second->size();
    }
}

bool
AsyncDemuxer::queuesFull() const {
    bool queuesFull = true;
    for (std::map<int, PacketQueuePtr>::const_iterator it = _myPacketQueues.begin(); it != _myPacketQueues.end(); ++it) {
        queuesFull &= (it->second->size() >= PACKET_QUEUE_SIZE);
    }
    return queuesFull;
}

}
