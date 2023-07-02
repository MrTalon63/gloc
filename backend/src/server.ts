import tcp from "./tcpServer";
import http from "./http";

async function main() {
	await tcp();
	await http();
}

main();
