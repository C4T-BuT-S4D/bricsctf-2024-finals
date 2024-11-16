package org.cbs.authrpc;

import com.zaxxer.hikari.HikariDataSource;

public class App {
    private static int portFromEnv(String key, int def) throws Exception {
        String portString = System.getenv().getOrDefault(key, Integer.toString(def));
        return Integer.parseInt(portString);
    }

    private static String envOr(String key, String def) {
        return System.getenv().getOrDefault(key, def);
    }

    public static void main(String[] args) throws Exception {

        TokenServer server = new TokenServer(
                portFromEnv("APP_PORT", 8980),
                new TokenService(new TokenStorage(getDataSource())));
        server.start();
        server.blockUntilShutdown();

    }

    private static HikariDataSource getDataSource() {
        HikariDataSource ds = new HikariDataSource();
        var url = String.format("jdbc:postgresql://%s:%s/%s", envOr("DB_HOST", "localhost"),
                envOr("DB_PORT", "5432"), envOr("DB_NAME", "qrsas"));
        ds.setJdbcUrl(url);
        ds.setUsername(envOr("DB_USER", "qrsas"));
        ds.setPassword(envOr("DB_PASS", "example"));
        return ds;
    }
}