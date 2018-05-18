
// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

///
/// \file   TObejct2JsonWorker.cxx
/// \author Vladimir Kosmala
///

// TObject2Json
#include "TObject2JsonWorker.h"
#include "TObject2JsonMySql.h"
#include "QualityControl/QcInfoLogger.h"

// ZMQ
#include "zmq.h"

// Boost
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace o2::quality_control::core;
using namespace std::string_literals;

namespace o2 {
namespace quality_control {
namespace tobject_to_json {

TObject2JsonWorker::TObject2JsonWorker(zmq::context_t &ctx, std::unique_ptr<Backend> backend)
  : mBackend(std::move(backend)),
    mCtx(ctx),
    mWorkerSocket(mCtx, ZMQ_DEALER),
    mThread(std::bind(&TObject2JsonWorker::start, this))
{}

TObject2JsonWorker::~TObject2JsonWorker() {
  mRun = false;
  QcInfoLogger::GetInstance() << "Ending worker" << infologger::endm;
  mThread.join();
}

void TObject2JsonWorker::start() {
  mRun = true;
  mWorkerSocket.setsockopt(ZMQ_RCVTIMEO, 200);
  mWorkerSocket.connect("inproc://backend");

  try {
    while (mRun) {
      std::string identity = socketReceive();
      if (identity.empty()) {
        // empty or timeout
        continue;
      }

      std::string payload = socketReceive();
      if (payload.empty()) {
        // empty or timeout
        continue;
      }

      std::string response = handleRequest(payload);

      // always success to send because of inproc communication
      socketSend(identity, response);
    }
  }
  catch (std::exception &e) {
    // Context was terminated
    QcInfoLogger::GetInstance() << "Closing worker" << infologger::endm;
  }
}

std::string TObject2JsonWorker::handleRequest(std::string request) {
  std::string agentName;
  std::string objectName;
  const auto slashIndex = request.find_first_of('/');

  if (std::string::npos == slashIndex) {
    QcInfoLogger::GetInstance() << "Wrong request request received: '" << request << "'" << infologger::endm;
    return std::string("{\"request\": \"") + request + std::string("\", ") +
      std::string("\"error\": 400, \"message\": \"Wrong request\", ") +
      std::string("\"why\": \"Ill-formed path, slash required\"}");
  }

  agentName = request.substr(0, slashIndex);
  objectName = request.substr(slashIndex + 1);

  try {
    // calls to getJsonObject implies the process to have a 'Interrupted system call', why?
    std::string result = mBackend->getJsonObject(agentName, objectName);

    return std::string("{\"request\": \"") +
      request +
      std::string("\", \"payload\": ") +
      result +
      std::string("}");
  } catch (const std::exception& error) {
    QcInfoLogger::GetInstance() <<
      "Internal error for request '" << request << "': '" << error.what() << "'" <<
      infologger::endm;
    return std::string("{\"request\": \"") + request + std::string("\", ") +
      std::string("\"error\": 500, \"message\": \"Internal error\", ") +
      std::string("\"why\": \"") + error.what() + std::string("\"}");
  }
}

//  Receive 0MQ string from socket and convert into string
std::string TObject2JsonWorker::socketReceive() {
  zmq::message_t message;
  bool received = mWorkerSocket.recv(&message);
  if (!received) {
    // timeout
    return std::string("");
  }
  return std::string(static_cast<char*>(message.data()), message.size());
}

//  Sends string as 0MQ string, as multipart non-terminal
bool TObject2JsonWorker::socketSend(const std::string & identity, const std::string & payload) {
  zmq::message_t identityMessage(identity.size());
  memcpy(identityMessage.data(), identity.data(), identity.size());

  mWorkerSocket.send(identityMessage, ZMQ_SNDMORE);

  zmq::message_t payloadMessage(payload.size());
  memcpy(payloadMessage.data(), payload.data(), payload.size());

  return mWorkerSocket.send(payloadMessage);
}

} // namespace tobject_to_json
} // namespace quality_control
} // namespace o2
