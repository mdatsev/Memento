////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2021 Ripose
//
// This file is part of Memento.
//
// Memento is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2 of the License.
//
// Memento is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Memento.  If not, see <https://www.gnu.org/licenses/>.
//
////////////////////////////////////////////////////////////////////////////////

#include "macospowereventhandler.h"

/**
 * Call back for handling a power event. Only cares about 'can sleep' messages.
 * @param h                The MacOsPowerEventHandler.
 * @param unused
 * @param messageType      The type of message to handle.
 * @param messageArguement Message argument, not to be directly modified.
 */
static void sleepWakeupCallBack(void *h,
                                io_service_t,
                                natural_t messageType,
                                void *messageArgument)
{
    if (messageType == kIOMessageCanSystemSleep)
    {
        MacOSPowerEventHandler *handler = (MacOSPowerEventHandler *)h;
        if (handler->getPreventSleep())
        {
            IOCancelPowerChange(
                handler->getRootPowerDomain(), (long)messageArgument
            );
        }
        else
        {
            IOAllowPowerChange(
                handler->getRootPowerDomain(), (long)messageArgument
            );
        }
    }
}

/**
 * Bootstraps into CFRunLoopRun().
 * @param handler The MacOSPowerEventHandler.
 */
static void eventLoop(MacOSPowerEventHandler *handler)
{
    IONotificationPortRef notifyPortRef;
    io_object_t           notifierObj;

    handler->setRunLoopRef(CFRunLoopGetCurrent());
    handler->setRootPowerDomain(
        IORegisterForSystemPower(
            handler, &notifyPortRef, sleepWakeupCallBack, &notifierObj
        )
    );
    if (handler->getRootPowerDomain() == 0)
    {
        std::cerr << "IORegisterForSystemPower failed" << std::endl;
        return;
    }
    CFRunLoopAddSource(
        CFRunLoopGetCurrent(),
        IONotificationPortGetRunLoopSource(notifyPortRef),
        kCFRunLoopCommonModes
    );
    CFRunLoopRun();
}

MacOSPowerEventHandler::MacOSPowerEventHandler()
    : preventSleep(false),
      runLoopRef(NULL)
{
    eventThread = new std::thread(eventLoop, this);
}

MacOSPowerEventHandler::~MacOSPowerEventHandler()
{
    CFRunLoopStop(runLoopRef);
    eventThread->join();
    delete eventThread;
}

void MacOSPowerEventHandler::setRunLoopRef(CFRunLoopRef ref)
{
    runLoopRef = ref;
}

void MacOSPowerEventHandler::setRootPowerDomain(const io_connect_t &r)
{
    rootPowerDomain = r;
}

io_connect_t MacOSPowerEventHandler::getRootPowerDomain() const
{
    return rootPowerDomain;
}

void MacOSPowerEventHandler::setPreventSleep(const bool value)
{
    preventSleep = value;
}

bool MacOSPowerEventHandler::getPreventSleep() const
{
    return preventSleep;
}
