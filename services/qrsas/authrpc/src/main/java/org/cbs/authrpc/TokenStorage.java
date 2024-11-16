package org.cbs.authrpc;

import com.google.common.collect.ImmutableList;

import javax.sql.DataSource;
import java.sql.SQLException;
import java.util.HashMap;
import java.util.List;
import java.util.Optional;

public class TokenStorage {
    public record TokenRecord(String token, String userId, String name) {

    }

    private final DataSource dataSource;
    public TokenStorage(DataSource dataSource) {
        this.dataSource = dataSource;
    }


    public void storeToken(String token, String userId, String name) throws SQLException {
        try (var conn = dataSource.getConnection()) {
            var statement = conn.prepareStatement("INSERT INTO tokens (token, user_id, name) values (?, ?, ?)");
            statement.setString(1, token);
            statement.setString(2, userId);
            statement.setString(3, name);
            statement.executeUpdate();
        }
    }

    public void deleteToken(String token) {
        try (var conn = dataSource.getConnection()) {
            var statement = conn.prepareStatement("DELETE FROM tokens WHERE token = ?");
            statement.setString(1, token);
            statement.executeUpdate();
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    public Optional<String> getUserId(String token) throws SQLException {
        try (var conn = dataSource.getConnection()) {
            var statement = conn.prepareStatement("SELECT user_id FROM tokens WHERE token = ?");
            statement.setString(1, token);
            var result = statement.executeQuery();
            return result.next() ? Optional.of(result.getString("user_id")) : Optional.empty();
        }
    }

    public ImmutableList<TokenRecord> listUserTokens(String userId) {
        try (var conn = dataSource.getConnection()) {
            var statement = conn.prepareStatement("SELECT token, name FROM tokens WHERE user_id = ?");
            statement.setString(1, userId);
            var result = statement.executeQuery();
            ImmutableList.Builder<TokenRecord> tokensBuilder = ImmutableList.builder();
            while (result.next()) {
                tokensBuilder.add(new TokenRecord(result.getString("token"), userId, result.getString("name")));
            }
            return tokensBuilder.build();
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }
}


