export default {
	mysql: {
		host: process.env.MYSQL_HOST || "localhost",
		port: process.env.MYSQL_PORT ? parseInt(process.env.MYSQL_PORT) : 3306,
		user: process.env.MYSQL_USER || "root",
		password: process.env.MYSQL_PASSWORD || "root",
		database: process.env.MYSQL_DATABASE || "test",
		connectionLimit: process.env.MYSQL_CONNECTION_LIMIT ? parseInt(process.env.MYSQL_CONNECTION_LIMIT) : 10,
	},
	tcpServer: {
		host: process.env.TCP_SERVER_HOST || "0.0.0.0",
		port: process.env.TCP_SERVER_PORT ? parseInt(process.env.TCP_SERVER_PORT) : 5000,
	},
	httpServer: {
		host: process.env.HTTP_SERVER_HOST || "0.0.0.0",
		port: process.env.HTTP_SERVER_PORT ? parseInt(process.env.HTTP_SERVER_PORT) : 3000,
	},
	encryption: {
		// This will be used to support AES256 encryption on the TCP server, for now it's just a placeholder
	},
};
