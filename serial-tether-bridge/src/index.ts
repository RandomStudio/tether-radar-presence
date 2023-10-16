import { SerialPort } from "serialport";
import { ReadlineParser } from "@serialport/parser-readline";

import defaults from "./defaults";
import parse from "parse-strings-in-object";
import rc from "rc";
import { getLogger } from "log4js";

const appName = "radar";

const config: typeof defaults = parse(rc(appName, defaults));

const logger = getLogger(appName);
logger.level = config.loglevel;

logger.info("started with config", config);

const handleData = (chunk: string) => {
  switch (chunk) {
    case "1": {
      logger.info("presence => TRUE");
      break;
    }
    case "0": {
      logger.info("presence => FALSE");
      break;
    }
    default: {
      logger.warn(`input unknown: "${chunk}"`);
    }
  }
};

const main = async () => {
  const port = new SerialPort({
    path: "/dev/tty.usbserial-022F04E1",
    baudRate: 115200,
  });
  const parser = port.pipe(new ReadlineParser({ delimiter: "\r\n" }));
  parser.on("data", handleData);
};

// ================================================
// Kick off main process here
main();
