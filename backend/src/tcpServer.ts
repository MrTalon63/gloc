import net from "node:net";

import config from "./config";
import { gsmPacket } from "./types";

async function tcp() {
	const server = net.createServer();

	server.on("connection", connectionHandler);

	server.listen(config.tcpServer.port, config.tcpServer.host, () => {
		console.log(`TCP server listening on ${config.tcpServer.host}:${config.tcpServer.port}`);
	});
}

async function connectionHandler(socket: net.Socket) {
	console.log(`New connection from ${socket.remoteAddress}:${socket.remotePort}`);

	socket.on("data", (data) => {
		console.log(`Received data from ${socket.remoteAddress}:${socket.remotePort}: ${data.toString("utf8")}`);
	});

	socket.on("close", () => {
		console.log(`Connection from ${socket.remoteAddress}:${socket.remotePort} closed`);
	});
}

async function packetParser(packet: string): Promise<gsmPacket> {
	const data = packet.split(",");

	const parsedPacket: gsmPacket = {
		encryption: data[0] === "LE" ? true : false,
		id: parseInt(data[1]),
		lat: parseFloat(data[2]),
		lng: parseFloat(data[3]),
		speed: parseFloat(data[4]),
		sats: parseInt(data[5]),
		signal: parseInt(data[6]),
		batteryVoltage: parseFloat(data[7]),
		supplyVoltage: parseFloat(data[8]),
	};

	return parsedPacket;
}

export default tcp;
