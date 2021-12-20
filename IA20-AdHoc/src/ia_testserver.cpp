/*
 * File: IA20-CommonLib-TestCases/src/testcases/TCURing.cpp
 *
 * Copyright (C) 2021, Albert Krzymowski
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include <ia20/uring/RingHandler.h>
#include <ia20/uring/IO/TCP/AsyncServer.h>
#include <ia20/uring/IO/ReadHandler.h>

#include <ia20/commonlib/net/conn/tcp/Peer.h>


#include <ia20/commonlib/net/conn/tcp/DefaultConnectionFactory.h>
#include <ia20/commonlib/net/conn/tcp/Client.h>
#include <ia20/commonlib/net/conn/tcp/Server.h>
#include <ia20/commonlib/net/conn/tcp/FileHandle.h>
#include <ia20/uring/IO/ReadHandler.h>
#include <ia20/uring/IO/WriteHandler.h>
#include <ia20/uring/IO/ShutdownHandler.h>

#include <unistd.h>
#include <string.h>

using namespace IA20;

using namespace URing;
using namespace URing::IO;

using namespace Net;
using namespace Net::Conn;
using namespace Net::Conn::TCP;

namespace MY{

static String CMSgCONNECT_Req("100C00044D5154540402003C0000");
static String CMSgCONNECT_Res("20020000");
//static String CMSgPUB_Req("30100009686F6D652F74656D703132333435");
static String CMSgPUB_Req("414243444546");
static String CMSgDISC_Req("E000");
/*************************************************************************/
class Server : public ReadHandler, public WriteHandler, public ShutdownHandler {
  public:
    Server(RingHandler *pRingHandler, FileHandle* pFileHandle):
      ReadHandler(pRingHandler, pFileHandle),
      WriteHandler(pRingHandler, pFileHandle),
      ShutdownHandler(pRingHandler, pFileHandle),
      ptrFileHandle(pFileHandle){
        WriteHandler::iovec.iov_len  = CStrData.length();
        WriteHandler::iovec.iov_base = malloc(4096);
        memcpy(WriteHandler::iovec.iov_base, CStrData.c_str(), WriteHandler::iovec.iov_len);
        strData.reserve(10000);
        ReadHandler::iovec.iov_len = 702;
      };

  static const String CStrData;

  String strData;

  size_t iMessages = 0;

  protected:

  void sendAndRead(const String& msg){
      MiscTools::HexToBinary(msg, WriteHandler::iovec.iov_base, 4000);
      WriteHandler::iovec.iov_len = msg.length()/2;
      WriteHandler::prepare();
      ReadHandler::prepare();
  }

  int iSkip = 0;
  String strLastData;
  size_t iTotalLen = 0;
  virtual void handleRead(off_t iDataLen){
     iTotalLen += iDataLen;
    //std::cout<<"Got data: ["<<iDataLen<<"]"<<std::endl;
      strData = MiscTools::BinarytoHex(ReadHandler::iovec.iov_base,iDataLen);
      //std::cout<<strData<<std::endl;
// 2F74656D703132333435
        if(strData == CMSgCONNECT_Req){
        //if(strData.rfind("1",0) == 0){
          std::cout<<"Connect"<<std::endl;
          sendAndRead(CMSgCONNECT_Res);
      }else if(strData.find(CMSgPUB_Req) == 2*iSkip){
          //std::cout<<"Publish"<<std::endl;
          iMessages += (iDataLen - iSkip) / 18;
          iSkip = 18 - (iDataLen - iSkip) % 18;
          if(iSkip == 18)
            iSkip = 0;
           else
            iMessages++;
         // std::cout<<"Publish:"<<iDataLen<<", "<<iMessages<<", "<<iSkip<<std::endl;
        //  std::cout<<strData<<std::endl<<std::endl;
          strLastData = strData;
          ReadHandler::prepare();
      }else if(strData == CMSgDISC_Req || iDataLen == 0){
          std::cout<<"Disconnect"<<std::endl;
          ShutdownHandler::prepare(SHUT_RDWR);
      }else{
          if(iDataLen == 0){
              ShutdownHandler::prepare(SHUT_RDWR);
          }else{
//             std::cout<<"unknown: "<<iSkip<<":"<<iDataLen<<","<<strData.find(CMSgPUB_Req)<<","<<strData<<std::endl;
  //           std::cout<<strLastData<<std::endl<<std::endl;
            ReadHandler::prepare();
          }
      }
  }

  virtual void handleWrite(off_t iDataLen){
      //std::cout<<"Written: ["<<iDataLen<<"]"<<std::endl;
  }

  virtual void handleShutdown(int iResult){
      std::cout<<"Shutdown: ["<<iResult<<"], msgs: "<<iMessages<<", "<<iTotalLen<<std::endl;
  }

  std::unique_ptr<FileHandle> ptrFileHandle;
};

const String Server::CStrData("123456789");

/*************************************************************************/
class Acceptor : public IO::TCP::AsyncServer::Acceptor {
  public:
    Acceptor(IO::TCP::AsyncServer* pAsyncServer, RingHandler *pRingHandler):
      IO::TCP::AsyncServer::Acceptor(pRingHandler, pAsyncServer){};

  protected:
    virtual void handleImpl(Net::Conn::TCP::FileHandle* pFileHandle){
      prepare();

      std::unique_ptr<Server> ptrServer(new Server(pRingHandler, pFileHandle));
      ptrServer->ReadHandler::prepare();
      lstServers.push_back(std::move(ptrServer));
  }

 ~Acceptor(){
     for(std::list< std::unique_ptr<Server> >::iterator it = lstServers.begin();
         it != lstServers.end(); it++){
        (*it).reset();
     }
 }
  std::list< std::unique_ptr<Server> > lstServers;
};

}

using namespace MY;

/*************************************************************************/
int main(){

    SYS::Signal::ThreadRegistration tr;
    try{

    std::unique_ptr<URing::RingHandler>            ptrRingHandler;

    ptrRingHandler.reset(new RingHandler);

    Net::Conn::TCP::Peer peer("0.0.0.0", 55556);

    std::unique_ptr<IO::TCP::AsyncServer> ptrAsyncServer(new IO::TCP::AsyncServer(peer, DefaultConnectionFactory::GetInstance()));

    std::unique_ptr<IO::TCP::AsyncServer::Acceptor>
         ptrAcceptor(new Acceptor(ptrAsyncServer.get(), ptrRingHandler.get()));

      ptrAcceptor->prepare();
        ptrRingHandler->handle();
    }catch(Exception& e){
        e.printToStream(std::cerr);
    }

};



/*************************************************************************/
