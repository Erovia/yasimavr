/*
 * arch_mega0_port.cpp
 *
 *	Copyright 2021 Clement Savergne <csavergne@yahoo.com>

 	This file is part of yasim-avr.

	yasim-avr is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	yasim-avr is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with yasim-avr.  If not, see <http://www.gnu.org/licenses/>.
 */

//=======================================================================================

#include "arch_mega0_port.h"
#include "arch_mega0_io.h"
#include "arch_mega0_io_utils.h"
#include "core/sim_device.h"
#include <cstddef>


//=======================================================================================

#define PORT_REG_ADDR(reg) \
	(m_config.reg_base_port + offsetof(PORT_t, reg))

#define PORT_REG_OFS(reg) \
	offsetof(PORT_t, reg)

#define VPORT_REG_ADDR(reg) \
	(m_config.reg_base_vport + offsetof(VPORT_t, reg))

#define VPORT_REG_OFS(reg) \
	offsetof(VPORT_t, reg)


//=======================================================================================

AVR_ArchMega0_Port::AVR_ArchMega0_Port(char name, const AVR_ArchMega0_PortConfig& config)
:AVR_IO_Port(name)
,m_config(config)
,m_port_value(0)
,m_dir_value(0)
{}

bool AVR_ArchMega0_Port::init(AVR_Device& device)
{
	bool status = AVR_IO_Port::init(device);

	add_ioreg(PORT_REG_ADDR(DIR));
	add_ioreg(PORT_REG_ADDR(DIRSET));
	add_ioreg(PORT_REG_ADDR(DIRCLR));
	add_ioreg(PORT_REG_ADDR(DIRTGL));
	add_ioreg(PORT_REG_ADDR(OUT));
	add_ioreg(PORT_REG_ADDR(OUTSET));
	add_ioreg(PORT_REG_ADDR(OUTCLR));
	add_ioreg(PORT_REG_ADDR(OUTTGL));
	add_ioreg(PORT_REG_ADDR(IN));
	add_ioreg(PORT_REG_ADDR(INTFLAGS));
	//PORTCTRL not implemented
	add_ioreg(PORT_REG_ADDR(PIN0CTRL), PORT_ISC_gm | PORT_PULLUPEN_bm);
	add_ioreg(PORT_REG_ADDR(PIN1CTRL), PORT_ISC_gm | PORT_PULLUPEN_bm);
	add_ioreg(PORT_REG_ADDR(PIN2CTRL), PORT_ISC_gm | PORT_PULLUPEN_bm);
	add_ioreg(PORT_REG_ADDR(PIN3CTRL), PORT_ISC_gm | PORT_PULLUPEN_bm);
	add_ioreg(PORT_REG_ADDR(PIN4CTRL), PORT_ISC_gm | PORT_PULLUPEN_bm);
	add_ioreg(PORT_REG_ADDR(PIN5CTRL), PORT_ISC_gm | PORT_PULLUPEN_bm);
	add_ioreg(PORT_REG_ADDR(PIN6CTRL), PORT_ISC_gm | PORT_PULLUPEN_bm);
	add_ioreg(PORT_REG_ADDR(PIN7CTRL), PORT_ISC_gm | PORT_PULLUPEN_bm);

	register_interrupt(m_config.iv_port, this);

	return status;
}

void AVR_ArchMega0_Port::reset()
{
	AVR_IO_Port::reset();
	m_port_value = 0;
	m_dir_value = 0;
}

void AVR_ArchMega0_Port::ioreg_read_handler(reg_addr_t addr)
{
	if (addr == VPORT_REG_ADDR(IN))
		write_ioreg(addr, read_ioreg(PORT_REG_ADDR(IN)));
}

void AVR_ArchMega0_Port::ioreg_write_handler(reg_addr_t addr, const ioreg_write_t& data)
{
	if (addr >= m_config.reg_base_port && addr < (m_config.reg_base_port + sizeof(PORT_t))) {
		reg_addr_t reg_ofs = addr - m_config.reg_base_port;
		uint8_t value_masked = data.value & pin_mask();
		switch(reg_ofs) {
		case PORT_REG_OFS(DIR):
			m_dir_value = value_masked;
			write_ioreg(addr, m_dir_value);
			break;

		case PORT_REG_OFS(DIRSET):
			m_dir_value |= value_masked;
			write_ioreg(PORT_REG_ADDR(DIR), m_dir_value);
			write_ioreg(addr, 0x00);
			break;

		case PORT_REG_OFS(DIRCLR):
			m_dir_value &= ~value_masked;
			write_ioreg(PORT_REG_ADDR(DIR), m_dir_value);
			write_ioreg(addr, 0x00);
			break;

		case PORT_REG_OFS(DIRTGL):
			m_dir_value ^= value_masked;
			write_ioreg(PORT_REG_ADDR(DIR), m_dir_value);
			write_ioreg(addr, 0x00);
			break;

		case PORT_REG_OFS(OUT):
			m_port_value = value_masked;
			write_ioreg(PORT_REG_ADDR(OUT), m_port_value);
			break;

		case PORT_REG_OFS(OUTSET):
			m_port_value |= value_masked;
			write_ioreg(PORT_REG_ADDR(OUT), m_port_value);
			write_ioreg(addr, 0x00);
			break;

		case PORT_REG_OFS(OUTCLR):
			m_port_value &= ~value_masked;
			write_ioreg(PORT_REG_ADDR(OUT), m_port_value);
			write_ioreg(addr, 0x00);
			break;

		case PORT_REG_OFS(OUTTGL):
			m_port_value ^= value_masked;
			write_ioreg(PORT_REG_ADDR(OUT), m_port_value);
			write_ioreg(addr, 0x00);
			break;

		case PORT_REG_OFS(INTFLAGS): {
			uint8_t intflags = read_ioreg(addr) & ~data.value;
			write_ioreg(addr, intflags);
			if (!intflags)
				cancel_interrupt(m_config.iv_port);
		} break;

		case PORT_REG_OFS(PIN0CTRL):
		case PORT_REG_OFS(PIN1CTRL):
		case PORT_REG_OFS(PIN2CTRL):
		case PORT_REG_OFS(PIN3CTRL):
		case PORT_REG_OFS(PIN4CTRL):
		case PORT_REG_OFS(PIN5CTRL):
		case PORT_REG_OFS(PIN6CTRL):
		case PORT_REG_OFS(PIN7CTRL): {
			uint8_t bitnum = addr - PORT_REG_OFS(PIN0CTRL);
			if ((pin_mask() >> bitnum) & 0x01) {
				bitmask_t rb_isc = DEF_BITMASK_F(PORT_ISC);
				uint8_t isc = rb_isc.extract(data.value);

				if (isc == PORT_ISC_LEVEL_gc && !test_ioreg(PORT_REG_OFS(IN), bitnum))
					raise_interrupt(m_config.iv_port);
				else
					cancel_interrupt(m_config.iv_port);
			}
		} break;

		default: break;
		}

		update_pin_states();

	} else {

		reg_addr_t reg_ofs = addr - m_config.reg_base_vport;
		switch(reg_ofs) {
		case VPORT_REG_OFS(DIR):
			ioreg_write_handler(PORT_REG_ADDR(DIR), data);
			break;

		case VPORT_REG_OFS(OUT):
			ioreg_write_handler(PORT_REG_ADDR(OUT), data);
			break;

		case VPORT_REG_OFS(INTFLAGS):
			ioreg_write_handler(PORT_REG_ADDR(INTFLAGS), data);
			break;

		default: break;
		}
	}
}

void AVR_ArchMega0_Port::update_pin_states()
{
	AVR_Pin::State state;
	uint8_t valdir = m_dir_value;
	uint8_t valport = m_port_value;
	uint8_t pinmask = pin_mask();

	for (int i = 0; i < 8; ++i) {
		if (pinmask && 0x01) {
			uint8_t pin_cfg = read_ioreg(m_config.reg_base_port + 0x10 + i);
			if (valdir & 0x01) {
				if (valport & 0x01)
					state = AVR_Pin::State_High;
				else
					state = AVR_Pin::State_Low;
			} else {
				if (BITSET(pin_cfg, PORT_PULLUPEN_bp))
					state = AVR_Pin::State_PullUp;
				else
					state = AVR_Pin::State_Floating;
			}
			set_pin_internal_state(i, state);
		}

		valdir >>= 1;
		valport >>= 1;
		pinmask >>= 1;
	}
}

void AVR_ArchMega0_Port::pin_state_changed(uint8_t num, AVR_Pin::State state)
{
	AVR_IO_Port::pin_state_changed(num, state);
	if (state == AVR_Pin::State_Shorted) return;

	//Extract the current and new pin boolean state and if there's no change, we return
	bool curr_value = test_ioreg(PORT_REG_ADDR(IN), num);
	bool new_value = (state == AVR_Pin::State_High ? 1 : 0);
	if (new_value == curr_value) return;

	//Read the Input Sense Config field of the pin control register
	uint8_t pin_cfg = read_ioreg(m_config.reg_base_port + 0x10 + num);
	bitmask_t rb_isc = DEF_BITMASK_F(PORT_ISC);
	uint8_t isc = rb_isc.extract(pin_cfg);

	//If ISC is Input Disabled, we can ignore the change
	if (isc == PORT_ISC_INPUT_DISABLE_gc) return;

	//Save the new pin state in the IN register, invert it if required
	write_ioreg(PORT_REG_ADDR(IN), num, new_value);

	//Check if we need to raise the interrupt
	bool raise_int = (isc == PORT_ISC_RISING_gc && new_value) ||
					 (isc == PORT_ISC_FALLING_gc && !new_value) ||
					 (isc == PORT_ISC_LEVEL_gc && !new_value) ||
					 (isc == PORT_ISC_BOTHEDGES_gc);

	if (raise_int && !test_ioreg(PORT_REG_ADDR(INTFLAGS), num)) {
		set_ioreg(PORT_REG_ADDR(INTFLAGS), num);
		raise_interrupt(m_config.iv_port);
	}
}
