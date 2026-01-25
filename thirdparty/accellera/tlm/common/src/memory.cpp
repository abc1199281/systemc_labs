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
//=============================================================================
///  @file memory.cpp
//
///  @brief Implement memory functionality
//
///  @details
///     This class implements the memory functionality (read, write, etc.)
///     and is used by all of the targets in the examples
//
//==============================================================================
//
//  Original Authors:
//    Jack Donovan, ESLX
//
//==============================================================================

// Modified by PO-WEI Huang in 2026 for SystemC learning purposes.    

#include "tlm/memory.h"
#include <iostream>

using namespace sc_core;

static const char *filename = "memory.cpp"; ///< filename for reporting

memory::memory
(
    const uint32_t ID,
    sc_core::sc_time   read_delay,
    sc_core::sc_time   write_delay,
    uint64_t      memory_size,
    uint32_t      memory_width
)
: m_ID              (ID)
, m_read_delay      (read_delay)
, m_write_delay     (write_delay)
, m_memory_size     (memory_size)
, m_memory_width    (memory_width)
, m_previous_warning(false)
{
    /// Allocate and initalize an array for the target's memory
    m_memory.resize(m_memory_size, 0);

    /// clear memory
    std::fill(m_memory.begin(), m_memory.end(), 0);

    sc_assert(m_memory_width > 0);
    sc_assert(m_memory_size % m_memory_width == 0);

    if ( m_memory_width > m_memory_size )
    {
        std::cout << "Target: " << m_ID
            <<" memory width is bigger than memory size";
        
    }
} // end Constructor

tlm::tlm_response_status memory::check_address(tlm::tlm_generic_payload  &gp)
{
    sc_dt::uint64    address   = gp.get_address();     // memory address
    unsigned  int     length   = gp.get_data_length(); // data length

    if ( address >= m_memory_size )
    {
        std::cout << "Target: " << m_ID
            << " Address out of range";
        return tlm::TLM_ADDRESS_ERROR_RESPONSE;
    }

    if ( (address + length) > m_memory_size )
    {
        std::cout << "Target: " << m_ID
            << " Data length extends beyond memory size";
        return tlm::TLM_BURST_ERROR_RESPONSE;
    }

    return tlm::TLM_OK_RESPONSE;
}

void memory::operation
(
    tlm::tlm_generic_payload  &gp,
    sc_core::sc_time          &delay_time   ///< transaction delay
)
{
    /// Access the required attributes from the payload
    sc_dt::uint64    address   = gp.get_address();     // memory address
    unsigned char*   data      = gp.get_data_ptr();    // data pointer
    unsigned int     length    = gp.get_data_length(); // data length
    tlm::tlm_command command   = gp.get_command();     // memory command

    tlm::tlm_response_status response_status = check_address(gp);

    if (gp.get_byte_enable_ptr())
    {
        gp.set_response_status(tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE);
    }
    else if (gp.get_streaming_width() != gp.get_data_length())
    {
        gp.set_response_status(tlm::TLM_BURST_ERROR_RESPONSE);
    }

    if (response_status != tlm::TLM_OK_RESPONSE)
    {
        gp.set_response_status(response_status);
        return;
    }

    switch (command)
    {
        case tlm::TLM_READ_COMMAND:
            for (unsigned int i = 0; i < length; i++)
            {
                data[i] = m_memory[address++];         // move the data to memory
            }
            delay_time = delay_time + m_read_delay;
            break;
        case tlm::TLM_WRITE_COMMAND:
            // Handle write command
            for (unsigned int i = 0; i < length; i++)
            {
                m_memory[address++] = data[i];     // move the data to memory
            }
            delay_time = delay_time + m_write_delay;
            break;
        default:
            if(m_previous_warning == false)
            {
                std::cout << "Target: " << m_ID
                    << " Unsupported GP command extension";
                m_previous_warning = true;
            }
            break;
    }

    gp.set_response_status(tlm::TLM_OK_RESPONSE);

    return;
}