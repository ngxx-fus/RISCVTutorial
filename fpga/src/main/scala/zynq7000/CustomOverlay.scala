package chipyard.fpga.zynq7000

import chisel3.Clock
import freechips.rocketchip.diplomacy.{AddressSet, BundleBridgeSource, InModuleBody, LazyModule, ValName}
import org.chipsalliance.cde.config.Field
import sifive.fpgashells.clocks.{ClockGroup, ClockSinkNode, ClockSinkParameters, ClockSourceNode}
import sifive.fpgashells.shell.xilinx.{GPIOXilinxPlacedOverlay, JTAGDebugXilinxPlacedOverlay, LEDXilinxPlacedOverlay, SDIOXilinxPlacedOverlay, SPIFlashXilinxPlacedOverlay, SingleEndedClockInputXilinxPlacedOverlay, UARTXilinxPlacedOverlay}
import sifive.fpgashells.shell.{CTSResetDesignInput, CTSResetPlacedOverlay, CTSResetShellInput, CTSResetShellPlacer, ClockInputDesignInput, ClockInputShellInput, ClockInputShellPlacer, DDRDesignInput, DDROverlayOutput, DDRPlacedOverlay, DDRShellInput, DDRShellPlacer, GPIODesignInput, GPIOShellInput, GPIOShellPlacer, IOPin, JTAGDebugDesignInput, JTAGDebugShellInput, JTAGDebugShellPlacer, LEDDesignInput, LEDShellInput, LEDShellPlacer, SPIDesignInput, SPIFlashDesignInput, SPIFlashShellInput, SPIFlashShellPlacer, SPIShellInput, SPIShellPlacer, UARTDesignInput, UARTShellInput, UARTShellPlacer}

/* =============================================================
============================ Clock =============================
===============================================================*/
class SysClockArtyPlacedOverlay(val shell: Zynq7000ShellCustomOverlays, name: String, val designInput: ClockInputDesignInput, val shellInput: ClockInputShellInput)
  extends SingleEndedClockInputXilinxPlacedOverlay(name, designInput, shellInput)
{
  val node = shell { ClockSourceNode(freqMHz = 100, jitterPS = 50) }

  shell { InModuleBody {
    val clk: Clock = io
    shell.xdc.addPackagePin(clk, "E3")
    shell.xdc.addIOStandard(clk, "LVCMOS33")
  } }
}
class SysClockArtyShellPlacer(val shell: Zynq7000ShellCustomOverlays, val shellInput: ClockInputShellInput)(implicit val valName: ValName)
  extends ClockInputShellPlacer[Zynq7000ShellCustomOverlays] {
  def place(designInput: ClockInputDesignInput) = new SysClockArtyPlacedOverlay(shell, valName.name, designInput, shellInput)
}


/* =============================================================
============================ CTS =============================
===============================================================*/
class CTSResetArtyPlacedOverlay(val shell: Zynq7000ShellCustomOverlays, name: String, val designInput: CTSResetDesignInput, val shellInput: CTSResetShellInput)
  extends CTSResetPlacedOverlay(name, designInput, shellInput)
class CTSResetArtyShellPlacer(val shell: Zynq7000ShellCustomOverlays, val shellInput: CTSResetShellInput)(implicit val valName: ValName)
  extends CTSResetShellPlacer[Zynq7000ShellCustomOverlays] {
  def place(designInput: CTSResetDesignInput) = new CTSResetArtyPlacedOverlay(shell, valName.name, designInput, shellInput)
}

/* =============================================================
======================= UART =============================
===============================================================*/
class UARTArtyPlacedOverlay(val shell: Zynq7000ShellCustomOverlays, name: String, val designInput: UARTDesignInput, val shellInput: UARTShellInput)
  extends UARTXilinxPlacedOverlay(name, designInput, shellInput, false)
{
  shell { InModuleBody {
    val packagePinsWithPackageIOs = Seq(
      ("A9", IOPin(io.rxd)),
      ("D10", IOPin(io.txd)))

    packagePinsWithPackageIOs foreach { case (pin, io) => {
      shell.xdc.addPackagePin(io, pin)
      shell.xdc.addIOStandard(io, "LVCMOS33")
      shell.xdc.addIOB(io)
    } }
  } }
}
class UARTArtyShellPlacer(val shell: Zynq7000ShellCustomOverlays, val shellInput: UARTShellInput)(implicit val valName: ValName)
  extends UARTShellPlacer[Zynq7000ShellCustomOverlays] {
  def place(designInput: UARTDesignInput) = new UARTArtyPlacedOverlay(shell, valName.name, designInput, shellInput)
}


/* =============================================================
============================= JTAG =============================
===============================================================*/
// PMOD JD used for JTAG
class JTAGDebugArtyPlacedOverlay(val shell: Zynq7000ShellCustomOverlays, name: String, val designInput: JTAGDebugDesignInput, val shellInput: JTAGDebugShellInput)
  extends JTAGDebugXilinxPlacedOverlay(name, designInput, shellInput)
{
  shell { InModuleBody {
    shell.sdc.addClock("JTCK", IOPin(io.jtag_TCK), 10)
    shell.sdc.addGroup(clocks = Seq("JTCK"))
    shell.xdc.clockDedicatedRouteFalse(IOPin(io.jtag_TCK))
    val packagePinsWithPackageIOs = Seq(
      ("D4", IOPin(io.jtag_TDI)),  //pin JD-0
      ("D3", IOPin(io.jtag_TMS)),  //pin JD-1
      ("F4", IOPin(io.jtag_TCK)),  //pin JD-2
      ("F3", IOPin(io.jtag_TDO)),  //pin JD-3
      ("H2", IOPin(io.srst_n)))

    packagePinsWithPackageIOs foreach { case (pin, io) => {
      shell.xdc.addPackagePin(io, pin)
      shell.xdc.addIOStandard(io, "LVCMOS33")
      shell.xdc.addPullup(io)
    } }
  } }
}

class JTAGDebugArtyShellPlacer(val shell: Zynq7000ShellCustomOverlays, val shellInput: JTAGDebugShellInput)(implicit val valName: ValName)
  extends JTAGDebugShellPlacer[Zynq7000ShellCustomOverlays] {
  def place(designInput: JTAGDebugDesignInput) = new JTAGDebugArtyPlacedOverlay(shell, valName.name, designInput, shellInput)
}

/* =============================================================
============================= SDIO =============================
===============================================================*/
class SDIOArtyPlacedOverlay(val shell: Zynq7000ShellCustomOverlays, name: String, val designInput: SPIDesignInput, val shellInput: SPIShellInput)
  extends SDIOXilinxPlacedOverlay(name, designInput, shellInput)
{
  shell { InModuleBody {
    val packagePinsWithPackageIOs = Seq(("D12", IOPin(io.spi_clk)),
      ("B11", IOPin(io.spi_cs)),
      ("A11", IOPin(io.spi_dat(0))),
      ("D13", IOPin(io.spi_dat(1))),
      ("B18", IOPin(io.spi_dat(2))),
      ("G13", IOPin(io.spi_dat(3))))

    packagePinsWithPackageIOs foreach { case (pin, io) => {
      shell.xdc.addPackagePin(io, pin)
      shell.xdc.addIOStandard(io, "LVCMOS33")
      shell.xdc.addIOB(io)
    } }
    packagePinsWithPackageIOs drop 1 foreach { case (pin, io) => {
      shell.xdc.addPullup(io)
    } }
  } }
}
class SDIOArtyShellPlacer(val shell: Zynq7000ShellCustomOverlays, val shellInput: SPIShellInput)(implicit val valName: ValName)
  extends SPIShellPlacer[Zynq7000ShellCustomOverlays] {
  def place(designInput: SPIDesignInput) = new SDIOArtyPlacedOverlay(shell, valName.name, designInput, shellInput)
}

/* =============================================================
========================= GPIO =================================
===============================================================*/
class GPIOArtyPlacedOverlay(val shell: Zynq7000ShellCustomOverlays,
                               name: String,
                               val designInput: GPIODesignInput,
                               val shellInput: GPIOShellInput)
  extends GPIOXilinxPlacedOverlay(name, designInput, shellInput)
{
  shell { InModuleBody {
    val gpioLocations = List("H5", "J5", "T9", "T10",    // LEDs
                            "E1", "F6", "G6", "G4",     // RGB LEDs
                            "J4", "G3", "H4", "J2",
                            "J3", "K2", "H6", "K1",
                            "A8", "C11", "C10", "A10", // Switches
                            "D9", "C9", "B9", "B8")     // Buttons
    val iosWithLocs = io.gpio.zip(gpioLocations)
    val packagePinsWithPackageIOs = iosWithLocs.map { case (io, pin) => (pin, IOPin(io)) }

    packagePinsWithPackageIOs foreach { case (pin, io) => {
      shell.xdc.addPackagePin(io, pin)
      shell.xdc.addIOStandard(io, "LVCMOS33")
      shell.xdc.addIOB(io)
    } }
  } }
}

class GPIOArtyShellPlacer(val shell: Zynq7000ShellCustomOverlays,
                             val shellInput: GPIOShellInput)(implicit val valName: ValName)
  extends GPIOShellPlacer[Zynq7000ShellCustomOverlays] {
  def place(designInput: GPIODesignInput) = new GPIOArtyPlacedOverlay(shell, valName.name, designInput, shellInput)
}
