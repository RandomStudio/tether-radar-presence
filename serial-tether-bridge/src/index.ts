import { SerialPort } from "serialport";
import { ReadlineParser } from "@serialport/parser-readline";
import { Output, TetherAgent } from "tether-agent";
import { encode } from "@msgpack/msgpack";

import defaults from "./defaults";
import parse from "parse-strings-in-object";
import rc from "rc";
import { getLogger } from "log4js";

const appName = "radar";

const config: typeof defaults = parse(rc(appName, defaults));

const logger = getLogger(appName);
logger.level = config.loglevel;

logger.info("started with config", config);

const sendTetherMessage = async (output: Output, presence: boolean) => {
  const m = encode(presence);
  await output.publish(m, { qos: 2, retain: true });
};

const main = async () => {
  const agent = await TetherAgent.create(config.tetherRole, {
    ...config.tether,
  });
  const output = agent.createOutput("presence");

  const port = new SerialPort({ ...config.serial });
  const parser = port.pipe(new ReadlineParser({ delimiter: "\r\n" }));

  parser.on("data", async (chunk: string) => {
    switch (chunk) {
      case "1": {
        logger.info("presence => TRUE");
        sendTetherMessage(output, true);
        break;
      }
      case "0": {
        logger.info("presence => FALSE");
        sendTetherMessage(output, false);
        break;
      }
      default: {
        logger.warn(`input unknown: "${chunk}"`);
      }
    }
  });
};

// ================================================
// Kick off main process here
main();
