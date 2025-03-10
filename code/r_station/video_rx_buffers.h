#pragma once

#include "../base/base.h"
#include "../base/config.h"
#include "../base/models.h"
#include "../radio/radiopackets2.h"


//  [packet header][video segment header][video seg header important][video data][000]
//  | pPH          | pPHVS               | pPHVSImp                  |pActualVideoData
//  | pRawData ptr                       | pVideoData ptr
//                                       [    <- video block packet size ->          ]
//                                                                   [-vid size-]

typedef struct
{
   u8* pRawData;
   u8* pVideoData; // pointer inside pRawData
   t_packet_header* pPH; // pointer inside pRawData
   t_packet_header_video_segment* pPHVS; // pointer inside pRawData
   t_packet_header_video_segment_important* pPHVSImp; // pointer inside pRawData
   u32 uReceivedTime;
   u32 uRequestedTime; // if requested for retransmission
   bool bEmpty;
   bool bReconstructed;
   bool bOutputed;
}
type_rx_video_packet_info;

typedef struct
{
   type_rx_video_packet_info packets[MAX_TOTAL_PACKETS_IN_BLOCK];
   u32 uVideoBlockIndex;
   bool bEmpty;
   int iMaxReceivedDataPacketIndex;
   int iEndOfFramePacketIndex;
   u32 uReceivedTime;
   int iBlockDataSize;
   int iBlockDataPackets;
   int iBlockECPackets;
   int iRecvDataPackets;
   int iRecvECPackets;
   int iReconstructedECUsed;
}
type_rx_video_block_info;

typedef struct
{
   unsigned int decode_missing_packets_indexes[MAX_TOTAL_PACKETS_IN_BLOCK];
   unsigned int decode_ec_packets_indexes[MAX_TOTAL_PACKETS_IN_BLOCK];
   u8* p_decode_data_packets_pointers[MAX_TOTAL_PACKETS_IN_BLOCK];
   u8* p_decode_ec_packets_pointers[MAX_TOTAL_PACKETS_IN_BLOCK];
   unsigned int missing_packets_count;
} type_fec_info;


class VideoRxPacketsBuffer
{
   public:
      VideoRxPacketsBuffer(int iVideoStreamIndex, int iCameraIndex);
      virtual ~VideoRxPacketsBuffer();

      bool init(Model* pModel);
      bool uninit();
      void emptyBuffers(const char* szReason);
      
      // Returns true if the packet has the highest video block/packet index received (in order)
      bool checkAddVideoPacket(u8* pPacket, int iPacketLength);

      u32 getMaxReceivedVideoBlockIndex();
      int getMaxReceivedVideoBlockPacketIndex();

      int getBlocksCountInBuffer();
      type_rx_video_block_info* getVideoBlockInBuffer(int iStartPosition);
      type_rx_video_packet_info* getMarkFirstPacketToOutput(type_rx_video_block_info** ppOutputBlock, bool bPushIncompleteBlocks);
      int discardOldBlocks(u32 uCutOffTime);
      void resetFrameEndDetectedFlag();
      bool isFrameEndDetected();
      u32 getFrameEndDetectionTime();

   protected:

      bool _check_allocate_video_block_in_buffer(int iBufferIndex);
      void _empty_block_buffer_packet_index(int iBufferIndex, int iPacketIndex);
      void _empty_block_buffer_index(int iBufferIndex);
      void _empty_buffers(const char* szReason, t_packet_header* pPH, t_packet_header_video_segment* pPHVS);
      void _check_do_ec_for_video_block(int iBufferIndex);
      void _add_video_packet_to_buffer(int iBufferIndex, u8* pPacket, int iPacketLength);

      static int m_siVideoBuffersInstancesCount;
      bool m_bInitialized;
      int m_iInstanceIndex;
      int m_iVideoStreamIndex;
      int m_iCameraIndex;
      bool m_bFrameEndDetected;
      u32  m_uFrameEndDetectedTime;

      // Buffers state
      type_rx_video_block_info m_VideoBlocks[MAX_RXTX_BLOCKS_BUFFER];
      u8 m_TempVideoBuffer[MAX_PACKET_TOTAL_SIZE];

      int m_iTopBufferIndex;
      int m_iBottomBufferIndexToOutput;
      int m_iBottomPacketIndexToOutput;

      type_fec_info m_ECRxInfo;
};

