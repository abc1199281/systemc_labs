/*****************************************************************************

  Licensed to Accellera Systems Initiative Inc. (Accellera) under one or
  more contributor license agreements.  See the NOTICE file distributed
  with this work for additional information regarding copyright ownership.
  Accellera licenses this file to you under the Apache License, Version 2.0
  (the "License"); you may not use this file except in compliance with the
  License.  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
  implied.  See the License for the specific language governing
  permissions and limitations under the License.

 *****************************************************************************/
//==============================================================================
///  @file memory.h
//
///  @brief Object for isolationg memory operations from TLM "shell"
//
//==============================================================================
//
//  Original Authors:
//    Jack Donovan, ESLX
//
//==============================================================================

// Modified by PO-WEI Huang in 2026 for SystemC learning purposes.    

#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <vector>
#include <systemc>
#include <tlm>

class memory
{
    memory(const memory&) = delete;
    memory& operator=(const memory&) = delete;

public:
    // Initialize member variables, include allocating and initializing
    memory(
        const uint32_t ID, ///< initiator ID for messaging
        sc_core::sc_time   read_delay,
        sc_core::sc_time   write_delay,
        uint64_t      memory_size,
        uint32_t      memory_width
    );

    void operation(
        tlm::tlm_generic_payload  &gp,
        sc_core::sc_time          &delay_time   ///< transaction delay
    );

    void get_delay(
        tlm::tlm_generic_payload  &gp,
        sc_core::sc_time          &delay_time
    );

private:
    tlm::tlm_response_status check_address(
        tlm::tlm_generic_payload  &gp
    );

public:
    std::vector<uint8_t>     m_memory;
private:
    const uint32_t   m_ID;
    const sc_core::sc_time   m_read_delay;
    const sc_core::sc_time   m_write_delay;
    const uint64_t           m_memory_size;
    const uint32_t           m_memory_width;
    bool                     m_previous_warning;
};

#endif // __MEMORY_H__