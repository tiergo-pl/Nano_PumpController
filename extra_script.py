Import("env", "projenv")

env.AddPostAction("$BUILD_DIR/${PROGNAME}.elf",env.VerboseAction("avr-objdump -h -S $BUILD_DIR/${PROGNAME}.elf > $BUILD_DIR/${PROGNAME}.lss","Creating listing file - $BUILD_DIR/${PROGNAME}.lss"))
env.AddPostAction("$BUILD_DIR/${PROGNAME}.elf",env.VerboseAction("$OBJCOPY -j .eeprom  --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0  --no-change-warnings -O ihex $BUILD_DIR/${PROGNAME}.elf $BUILD_DIR/${PROGNAME}.eep || exit 0","Creating eeprom file - $BUILD_DIR/${PROGNAME}.eep"))
env.AddPostAction("$BUILD_DIR/${PROGNAME}.elf",env.VerboseAction("$OBJCOPY -j .eeprom  --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0  --no-change-warnings -O binary $BUILD_DIR/${PROGNAME}.elf $BUILD_DIR/${PROGNAME}.bin || exit 0","Creating eeprom file - $BUILD_DIR/${PROGNAME}.bin"))
