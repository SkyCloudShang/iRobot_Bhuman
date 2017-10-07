/**
 * @file Tools/BNTP.cpp
 *
 * Representations and functions for time synchronisation inside
 * the team. Implementation of parts of the Network Time Protocol.
 *
 * @author <a href="mailto:tlaue@uni-bremen.de">Tim Laue</a>
 * @author <A href="mailto:jesse@tzi.de">Jesse Richter-Klug</A>
 */

#include "BNTP.h"
#include "Platform/Time.h"

void BNTP::operator >> (BHumanMessage& m) const
{
  m.theBHULKsStandardMessage.ntpMessages.clear();
  // Process incoming NTP requests:
  while(!receivedNTPRequests.empty())
  {
    const BNTPRequest& request = receivedNTPRequests.back();
    B_HULKs::BNTPMessage bntpMessage;
    bntpMessage.receiver = request.sender;
    bntpMessage.requestOrigination = request.origination;
    bntpMessage.requestReceipt = request.receipt;
    m.theBHULKsStandardMessage.ntpMessages.emplace_back(bntpMessage);

    const_cast<RingBuffer<BNTPRequest, MAX_NUM_OF_NTP_PACKAGES>&>(receivedNTPRequests).pop_back();
  }

  // Send NTP requests to teammates?
  if((m.theBHULKsStandardMessage.requestsNTPMessage = theFrameInfo.time - lastNTPRequestSent >= NTP_REQUEST_INTERVAL))
    const_cast<unsigned&>(lastNTPRequestSent) = theFrameInfo.time;
}

void BNTP::operator<<(const BHumanMessage& m)
{
  const unsigned receiveTimeStamp = Time::getCurrentSystemTime();

  if(m.theBHULKsStandardMessage.requestsNTPMessage)
  {
    BNTPRequest ntpRequest;
    ntpRequest.sender = m.theBSPLStandardMessage.playerNum;
    ntpRequest.origination = m.theBHULKsStandardMessage.timestamp;
    ntpRequest.receipt = receiveTimeStamp;
    receivedNTPRequests.push_front(ntpRequest);
  }

  for(auto itr = m.theBHULKsStandardMessage.ntpMessages.cbegin(); itr != m.theBHULKsStandardMessage.ntpMessages.cend(); ++itr)
  {
    //Check, if this response belongs to an own request:
    if(itr->receiver == theRobotInfo.number)
    {
      SynchronizationMeasurementsBuffer::SynchronizationMeasurement t;
      t.roundTrip = int(receiveTimeStamp - itr->requestOrigination) - int(m.theBHULKsStandardMessage.timestamp - itr->requestReceipt);
      t.offset = int(itr->requestReceipt - itr->requestOrigination + m.theBHULKsStandardMessage.timestamp - receiveTimeStamp) / 2;
      timeSyncBuffers[m.theBSPLStandardMessage.playerNum].add(t);
    }
  }
}

void SynchronizationMeasurementsBuffer::add(const SynchronizationMeasurement& s)
{
  buffer.push_front(s);

  bestOffset = buffer[0].offset;
  int shortestRoundTrip = buffer[0].roundTrip;
  for(size_t i = 1; i < buffer.size(); ++i)
    if(buffer[i].roundTrip < shortestRoundTrip)
    {
      shortestRoundTrip = buffer[i].roundTrip;
      bestOffset = buffer[i].offset;
    }
}
