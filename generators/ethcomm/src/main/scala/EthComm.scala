package ethcomm

import chisel3._
import chisel3.util._
import org.chipsalliance.cde.config.{Config, Field, Parameters}
import freechips.rocketchip.diplomacy._
import freechips.rocketchip.tilelink._
import freechips.rocketchip.subsystem._

/*
 * @brief Configuration parameters for EthComm IP block.
 * @param address Base address mapped in system address space (default: 0x3000_0000).
 * @param size Memory-mapped aperture size in bytes (default: 1MB = 0x100000).
 */ 
case class EthCommParams(
    address: BigInt = 0x30000000L,
    size: BigInt    = 0x100000L
)

/*
 * @brief Config key to inject EthComm parameters via Parameters context.
 */ 
case object EthCommKey extends Field[Option[EthCommParams]](None)

/*
 * @brief TileLink Diplomatic LazyModule wrapper for EthComm slave device.
 * @param params Memory and addressing configuration.
 * @param beatBytes Bus data width in bytes (e.g., 4 bytes for 32-bit, 8 for 64-bit).
 */ 
class TLEthComm(params: EthCommParams, beatBytes: Int)(implicit p: Parameters) extends LazyModule {
    // Register TLManagerNode supporting Get, PutFull, and PutPartial operations
    val node = TLManagerNode(Seq(TLSlavePortParameters.v1(
        managers = Seq(TLSlaveParameters.v1(
            address            = Seq(AddressSet(params.address, params.size - 1)),
            resources          = new SimpleDevice("ethcomm", Seq("custom,ethcomm")).reg,
            regionType         = RegionType.UNCACHED,
            supportsGet        = TransferSizes(1, beatBytes),
            supportsPutPartial = TransferSizes(1, beatBytes),
            supportsPutFull    = TransferSizes(1, beatBytes),
            fifoId             = Some(0)
        )),
        beatBytes = beatBytes
    )))
    lazy val module = new TLEthCommModuleImp(this, params, beatBytes)
}

/*
 * @brief Hardware implementation module for TLEthComm with 1-cycle synchronous SRAM.
 * @param outer Parent LazyModule diplomatic node.
 * @param params Memory size and configuration parameters.
 * @param beatBytes Bus data width in bytes.
 */ 
class TLEthCommModuleImp(outer: TLEthComm, params: EthCommParams, beatBytes: Int) extends LazyModuleImp(outer) {
    // Acquire input TileLink port and edge metadata
    val (tl_in, edge) = outer.node.in.head

    // Total memory words based on configured beat size
    val numWords = (params.size / beatBytes).toInt
    val mem = SyncReadMem(numWords, Vec(beatBytes, UInt(8.W)))

    // Channel A request decoding
    val a = tl_in.a
    val addrMask = (params.size - 1).U
    val wordAddr = ((a.bits.address & addrMask) >> log2Ceil(beatBytes)).asUInt

    // Identify incoming transaction type
    val isPut = a.bits.opcode === TLMessages.PutFullData || a.bits.opcode === TLMessages.PutPartialData
    val isGet = a.bits.opcode === TLMessages.Get

    // Slice input data into byte vectors for selective byte-mask writes
    val wdataVec = VecInit(Seq.tabulate(beatBytes)(i => a.bits.data(8 * (i + 1) - 1, 8 * i)))

    // Control flow: write to memory when a valid Put transaction fires
    when(a.fire && isPut) {
        mem.write(wordAddr, wdataVec, a.bits.mask.asBools)
    }

    // Read synchronously from memory when a Get transaction fires
    val rdataVec = mem.read(wordAddr, a.fire && isGet)
    val rdataBits = Cat(rdataVec.reverse)

    // Pipeline response registers for Channel D
    val r_opcode = RegEnable(a.bits.opcode, a.fire)
    val r_source = RegEnable(a.bits.source, a.fire)
    val r_size   = RegEnable(a.bits.size,   a.fire)
    val d_valid  = RegInit(false.B)

    // Control flow: steering logic for response valid handshake
    when(a.fire) {
        d_valid := true.B
    }.elsewhen(tl_in.d.fire) {
        d_valid := false.B
    }

    // Throttle Channel A accept when Channel D response register is busy
    a.ready := !d_valid || tl_in.d.ready

    // Steer Channel D response signals back to master
    tl_in.d.valid := d_valid
    tl_in.d.bits  := Mux(
        r_opcode === TLMessages.Get,
        edge.AccessAck(r_source, r_size, rdataBits),
        edge.AccessAck(r_source, r_size)
    )

    // Control flow: tie off unused TL-C coherence channels if present
    if (edge.manager.anySupportAcquireB) {
        tl_in.b.ready := true.B
        tl_in.c.valid := false.B
        tl_in.e.ready := true.B
    }
}

/*
 * @brief Cake Pattern trait for Subsystem to instantiate and attach EthComm to pbus.
 */ 
trait CanHavePeripheryEthComm { this: BaseSubsystem =>
    // Query configuration key to check if EthComm is enabled
    val ethCommOpt = p(EthCommKey).map { params =>
        val ethComm = LazyModule(new TLEthComm(params, pbus.beatBytes))
        // Attach EthComm slave node to peripheral crossbar (pbus)
        pbus.coupleTo("ethcomm") {
            ethComm.node := TLFragmenter(pbus.beatBytes, pbus.blockBytes) := _
        }
        // Return instantiated LazyModule reference
        ethComm
    }
}

/*
 * @brief Cake Pattern implementation trait for module execution.
 */ 
trait CanHavePeripheryEthCommImp extends LazyModuleImp {
    val outer: CanHavePeripheryEthComm
}

/*
 * @brief Config fragment to enable EthComm IP block with customized parameters.
 * @param address Base address in memory map (default: 0x3000_0000).
 * @param size Memory region size in bytes (default: 1MB = 0x100000).
 */ 
class WithEthComm(address: BigInt = 0x30000000L, size: BigInt = 0x100000L) extends Config((site, here, up) => {
    case EthCommKey =>
        // Return parameter configuration object wrapped in Option
        Some(EthCommParams(address = address, size = size))
})