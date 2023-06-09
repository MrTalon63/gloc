import mysql from "mysql2/promise";
import config from "./config";

const pool = mysql.createPool({
	host: config.mysql.host,
	port: config.mysql.port,
	user: config.mysql.user,
	password: config.mysql.password,
	database: config.mysql.database,
	connectionLimit: config.mysql.connectionLimit,
});

export default pool;
