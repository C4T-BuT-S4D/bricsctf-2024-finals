import React, { useState } from 'react';
import { register, login } from '../api';
import { Button, TextField, Container, Typography } from '@mui/material';
import { useNavigate } from 'react-router-dom';
import { useError } from '../ErrorContext';

const Auth = () => {
  const { showError } = useError();
  const [formData, setFormData] = useState({ username: '', password: '' });
  const navigate = useNavigate(); // Initialize the useNavigate hook

  const handleChange = (e) => {
    setFormData({
      ...formData,
      [e.target.name]: e.target.value,
    });
  };

  const handleRegister = async () => {
    try {
      const response = await register(formData);
      if (response.data.status === 'success') {
        localStorage.setItem('token', response.data.message.token);
      } else {
        showError(response.data.message);
      }
    } catch (error) {
      showError('Registration failed. Please try again.');
    }
  };

  const handleLogin = async () => {
    try {
      const response = await login(formData);
      if (response.data.status === 'success') {
        localStorage.setItem('token', response.data.message.token);
        navigate('/notes'); // Redirect to the notes page on success
      } else {
        showError(response.data.message);
      }
    } catch (error) {
      showError('Login failed. Please try again.');
    }
  };

  return (
    <Container maxWidth="xs">
      <Typography variant="h4" component="h1" gutterBottom>
        Authentication
      </Typography>
      <TextField
        fullWidth
        label="Username"
        margin="normal"
        name="username"
        variant="outlined"
        onChange={handleChange}
      />
      <TextField
        fullWidth
        type="password"
        label="Password"
        margin="normal"
        name="password"
        variant="outlined"
        onChange={handleChange}
      />
      <Button variant="contained" color="primary" fullWidth onClick={handleRegister}>
        Register
      </Button>
      <Button variant="contained" color="secondary" fullWidth onClick={handleLogin}>
        Login
      </Button>
    </Container>
  );
};

export default Auth;
