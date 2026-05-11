# Building the Brain Behind an Electron Microscope

If you've ever seen those impossibly detailed images of a fly's eye or a virus particle, those came from an electron microscope. And if you've ever wondered how someone actually *controls* one of those machines — well, that's what this project is about.

---

## What even is an electron microscope?

Regular light microscopes use photons — light — to see things. The problem is that light has a physical limit to how small it can see. Once you go below a certain size (roughly the width of a bacterium), you're out of luck.

Electron microscopes get around this by using electrons instead of light. Electrons have a much smaller wavelength, which means you can image things at the nanometer scale — we're talking individual proteins, cell membranes, virus capsids. Stuff that was invisible to science for most of human history.

But with that precision comes a catch: everything has to be incredibly controlled. The electron beam needs to be at exactly the right voltage. The sample has to sit on a stage that can move in five different directions with micrometer accuracy. And the whole thing has to be inside a vacuum chamber, because electrons scatter off air molecules and you'd get nothing but noise.

One stray command, wrong parameter, or missed safety check and you either get a useless image or you damage the sample entirely.

---

## So what did I actually build?

I built the software layer that sits between a host computer and the microscope's control electronics — the thing that translates high-level commands like "move stage to position X" or "set beam voltage to 10 kilovolts" into actual hardware instructions, and handles all the safety logic in between.

It's written in C++ and structured the way real embedded firmware systems are: a host side that packages commands and sends them over a communication channel, and a control board side that receives, parses, validates, and executes those commands against the actual hardware subsystems.

Think of it like this: the host computer is the pilot, and the control board is the autopilot that actually knows what the plane can and can't do safely. You can ask for 99 kilovolts of beam voltage, but the system will reject it — the safe range is 0.1 to 30 kV. You can try to tilt the stage 90 degrees, but it'll tell you no — the physical limit is ±70 degrees. You can attempt to fire the beam without the vacuum being ready, and it will not let you, full stop.

That last one matters a lot. The beam cannot enable unless the vacuum system reports pressure below 0.001 Pascals. That's about 10 billion times lower than atmospheric pressure. You fire electrons into air, you destroy the sample and probably the instrument. So that check isn't a nice-to-have, it's a hard gate in the code.

---

## The parts that made it interesting

There are four main hardware subsystems the software manages:

**The beam controller** handles voltage, current, and focus distance. All three have tight physical bounds, and the beam state itself has to be tracked — you can't re-enable a beam that's already on, and you can't acquire images if the beam is off.

**The stage controller** moves the sample in five axes: X and Y translation across a 100mm span, Z height up to 40mm, and tilt up to 70 degrees in either direction. Every move gets validated against hardware limits before it executes. This isn't just error handling — in a real instrument, driving past physical travel limits can mechanically damage the stage.

**The vacuum system** simulates the pump-down cycle from atmospheric pressure to beam-safe levels. Real vacuum systems take minutes to reach operating pressure, and the software has to model that state transition accurately so the rest of the system knows when it's safe to proceed.

**The microscope controller** sits above all three and owns the overall system state machine: idle, ready, imaging, and emergency stop. The emergency stop state is a one-way door — once triggered, the only way out is an explicit reset command, and certain operations stay locked out even after reset until the system re-verifies safe conditions.

---

## The communication layer

Everything between the host and the control board goes through a custom wire protocol. Each message starts with a two-byte magic number (0x45 0x4D — 'E' and 'M' for electron microscope), followed by the message type, command ID, payload length, the actual payload data, and a 16-bit CRC checksum at the end.

The CRC is there to catch corruption. If a message arrives garbled — wrong byte flipped, truncated mid-transmission — the parser catches it and rejects the frame rather than executing a partially corrupted command. That matters especially for floating-point values like voltages and positions, where a single bad byte turns "10.0 kV" into something completely different.

The message parser is implemented as a finite state machine, processing one byte at a time. This mirrors how real embedded systems read serial data — you don't get a complete packet handed to you, you get a byte stream and you figure out where frames begin and end.
