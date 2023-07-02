import express from "express";

import httpRoutes from "./httpRouter/router";
import config from "./config";

async function http() {
	const app = express();

	app.get("/", (req, res) => {
		res.json({
			status: "OK",
			message: "Welcome to the GLoc API! This is the default, health check endpoint.",
		});
	});

	app.use("/api", httpRoutes);

	app.listen(config.httpServer.port, config.httpServer.host, () => {
		console.log(`HTTP server listening on ${config.httpServer.host}:${config.httpServer.port}`);
	});
}

export default http;
