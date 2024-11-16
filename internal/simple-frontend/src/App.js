import React from 'react';
import { BrowserRouter as Router, Route, Routes, Link } from 'react-router-dom';
import Auth from './components/Auth';
import Notes from './components/Notes';
import { AppBar, Toolbar, IconButton, Typography, Container, Button } from '@mui/material';
import NoteIcon from '@mui/icons-material/Note';

function App() {
  return (
    <Router>
      <AppBar position="static">
        <Toolbar>
          <IconButton edge="start" color="inherit" aria-label="menu" component={Link} to="/">
            <NoteIcon />
          </IconButton>
          <Typography variant="h6" style={{ flexGrow: 1 }}>
            Note App
          </Typography>
          <Button color="inherit" component={Link} to="/notes">
            Notes
          </Button>
          <Button color="inherit" component={Link} to="/">
            Login/Register
          </Button>
        </Toolbar>
      </AppBar>
      <Container style={{ marginTop: '2rem' }}>
        <Routes>
          <Route path="/" element={<Auth />} />
          <Route path="/notes" element={<Notes />} />
        </Routes>
      </Container>
    </Router>
  );
}

export default App;
