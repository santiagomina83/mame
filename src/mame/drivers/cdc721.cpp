// license:BSD-3-Clause
// copyright-holders:Robbbert
/************************************************************************************************************

Control Data Corporation CDC 721 Terminal (Viking)

2013-08-13 Skeleton


*************************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/ins8250.h"
//#include "machine/nvram.h"
#include "machine/z80ctc.h"
#include "video/tms9927.h"
#include "screen.h"


class cdc721_state : public driver_device
{
public:
	cdc721_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_chargen(*this, "chargen")
		, m_p_videoram(*this, "videoram")
	{ }

	void cdc721(machine_config &config);
private:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_PALETTE_INIT(cdc721);
	DECLARE_WRITE8_MEMBER(interrupt_mask_w);
	DECLARE_WRITE8_MEMBER(misc_w);
	DECLARE_WRITE8_MEMBER(lights_w);
	DECLARE_WRITE8_MEMBER(bank_select_w);

	void io_map(address_map &map);
	void mem_map(address_map &map);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	u8 m_flashcnt;

	required_device<cpu_device> m_maincpu;
	required_region_ptr<u8> m_p_chargen;
	required_shared_ptr<u8> m_p_videoram;
};

WRITE8_MEMBER(cdc721_state::interrupt_mask_w)
{
	logerror("%s: Interrupt mask = %02X\n", machine().describe_context(), data ^ 0xff);
}

WRITE8_MEMBER(cdc721_state::misc_w)
{
	logerror("%s: %d-column display selected\n", machine().describe_context(), BIT(data, 3) ? 132 : 80);
}

WRITE8_MEMBER(cdc721_state::lights_w)
{
	logerror("%s: Lights = %02X\n", machine().describe_context(), data ^ 0xff);
}

WRITE8_MEMBER(cdc721_state::bank_select_w)
{
	logerror("%s: Bank select = %02X\n", machine().describe_context(), data);
	//membank("bankr0")->set_entry(BIT(data, 3));
}

void cdc721_state::mem_map(address_map &map)
{
	map(0x0000, 0x4fff).rom().region("maincpu", 0x10000);
//  AM_RANGE(0x0000, 0x4fff) AM_READ_BANK("bankr0") AM_WRITE_BANK("bankw0")
	map(0x8000, 0xdfff).ram();
	map(0xe000, 0xffff).ram().share("videoram");
}

void cdc721_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x10, 0x1f).rw("crtc", FUNC(crt5037_device::read), FUNC(crt5037_device::write));
	map(0x20, 0x27).rw("uart1", FUNC(ins8250_device::ins8250_r), FUNC(ins8250_device::ins8250_w));
	map(0x30, 0x33).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x40, 0x47).rw("uart2", FUNC(ins8250_device::ins8250_r), FUNC(ins8250_device::ins8250_w));
	map(0x50, 0x50).w(this, FUNC(cdc721_state::lights_w));
	map(0x70, 0x70).w(this, FUNC(cdc721_state::bank_select_w));
	map(0x80, 0x87).rw("uart3", FUNC(ins8250_device::ins8250_r), FUNC(ins8250_device::ins8250_w));
	map(0x90, 0x97).rw("uart4", FUNC(ins8250_device::ins8250_r), FUNC(ins8250_device::ins8250_w));
}

static INPUT_PORTS_START( cdc721 )
INPUT_PORTS_END

void cdc721_state::machine_reset()
{
//  membank("bankr0")->set_entry(0);
//  membank("bankw0")->set_entry(0);
}

void cdc721_state::machine_start()
{
//  uint8_t *main = memregion("maincpu")->base();

//  membank("bankr0")->configure_entry(1, &main[0x14000]);
//  membank("bankr0")->configure_entry(0, &main[0x4000]);
//  membank("bankw0")->configure_entry(0, &main[0x4000]);
}

/* F4 Character Displayer */
static const gfx_layout cdc721_charlayout =
{
	8, 16,                  /* 8 x 16 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	/* y offsets */
	{ 0*8, 0x100*8, 0x200*8, 0x300*8, 0x400*8, 0x500*8, 0x600*8, 0x700*8,
	0x800*8, 0x900*8, 0xa00*8, 0xb00*8, 0xc00*8, 0xd00*8, 0xe00*8, 0xf00*8 },
	8                   /* every char takes 16 x 1 bytes */
};

static GFXDECODE_START( cdc721 )
	GFXDECODE_ENTRY( "chargen", 0x0000, cdc721_charlayout, 0, 1 )
GFXDECODE_END

PALETTE_INIT_MEMBER( cdc721_state, cdc721 )
{
	palette.set_pen_color(0, 0, 0, 0 ); /* Black */
	palette.set_pen_color(1, 0, 255, 0 );   /* Full */
	palette.set_pen_color(2, 0, 128, 0 );   /* Dimmed */
}

uint32_t cdc721_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t y,ra,chr,gfx,attr,pen;
	uint16_t sy=0,x;
	m_flashcnt++;

	for (y = 0; y < 30; y++)
	{
		uint16_t ma = m_p_videoram[y * 2] | m_p_videoram[y * 2 + 1] << 8;

		for (ra = 0; ra < 16; ra++)
		{
			uint16_t *p = &bitmap.pix16(sy++);

			for (x = 0; x < 160; x+=2)
			{
				pen = 1;
				chr = m_p_videoram[(x + ma) & 0x1fff];
				attr = m_p_videoram[(x + ma + 1) & 0x1fff];
				gfx = m_p_chargen[chr | (ra << 8) ];
				if (BIT(attr, 0))  // blank
					pen = 0;
				if (BIT(attr, 1) && (ra == 14)) // underline
					gfx = 0xff;
				if (BIT(attr, 4)) // dim
					pen = 2;
				if (BIT(attr, 2)) // rv
					gfx ^= 0xff;
				if (BIT(attr, 3) && BIT(m_flashcnt, 6)) // blink
					gfx = 0;

				/* Display a scanline of a character */
				*p++ = BIT(gfx, 0) ? pen : 0;
				*p++ = BIT(gfx, 1) ? pen : 0;
				*p++ = BIT(gfx, 2) ? pen : 0;
				*p++ = BIT(gfx, 3) ? pen : 0;
				*p++ = BIT(gfx, 4) ? pen : 0;
				*p++ = BIT(gfx, 5) ? pen : 0;
				*p++ = BIT(gfx, 6) ? pen : 0;
				*p++ = BIT(gfx, 7) ? pen : 0;
			}
		}
	}
	return 0;
}

static const z80_daisy_config cdc721_daisy_chain[] =
{
	{ "ctc" },
	{ nullptr }
};

MACHINE_CONFIG_START(cdc721_state::cdc721)
	// basic machine hardware
	MCFG_CPU_ADD("maincpu", Z80, 4000000)
	MCFG_CPU_PROGRAM_MAP(mem_map)
	MCFG_CPU_IO_MAP(io_map)
	MCFG_Z80_DAISY_CHAIN(cdc721_daisy_chain)

	//MCFG_NVRAM_0FILL_ADD("nvram") // MCM51L01C45 (256x4) + battery

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(cdc721_state, screen_update)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 479)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_PALETTE_ADD("palette", 3)
	MCFG_PALETTE_INIT_OWNER(cdc721_state, cdc721)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", cdc721)

	MCFG_DEVICE_ADD("crtc", CRT5037, 12.936_MHz_XTAL / 8)
	MCFG_TMS9927_CHAR_WIDTH(8)

	MCFG_DEVICE_ADD("ctc", Z80CTC, 4000000)
	MCFG_Z80CTC_INTR_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))

	MCFG_DEVICE_ADD("ppi", I8255A, 0)
	MCFG_I8255_OUT_PORTB_CB(WRITE8(cdc721_state, interrupt_mask_w))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(cdc721_state, misc_w))

	MCFG_DEVICE_ADD("uart1", INS8250, 1843200)
	MCFG_DEVICE_ADD("uart2", INS8250, 1843200)
	MCFG_DEVICE_ADD("uart3", INS8250, 1843200)
	MCFG_DEVICE_ADD("uart4", INS8250, 1843200)
MACHINE_CONFIG_END

ROM_START( cdc721 )
	ROM_REGION( 0x15000, "maincpu", 0 )
	ROM_LOAD( "66315359", 0x10000, 0x2000, CRC(20ff3eb4) SHA1(5f15cb14893d75a46dc66d3042356bb054d632c2) )
	ROM_LOAD( "66315361", 0x12000, 0x2000, CRC(21d59d09) SHA1(9c087537d68c600ddf1eb9b009cf458231c279f4) )
	ROM_LOAD( "66315360", 0x14000, 0x1000, CRC(feaa0fc5) SHA1(f06196553a1f10c07b2f7e495823daf7ea26edee) )
	//ROM_FILL(0x14157,1,0xe0)

	ROM_REGION( 0x1000, "keyboard", 0 )
	ROM_LOAD( "66307828", 0x0000, 0x1000, CRC(ac97136f) SHA1(0d280e1aa4b9502bd390d260f83af19bf24905cd) ) // keyboard lookup

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "66315039", 0x0000, 0x1000, CRC(5c9aa968) SHA1(3ec7c5f25562579e6ed3fda7562428ff5e6b9550) )
ROM_END

COMP( 1981, cdc721, 0, 0, cdc721, cdc721, cdc721_state, 0, "Control Data Corporation", "721 Display Terminal", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
