import React, { useEffect, useState } from 'react';
import { fetchNotes, fetchNote, createNote } from '../api';
import { Container, List, ListItem, ListItemText, Typography, Button, TextField, Grid, CircularProgress } from '@mui/material';
import { useError } from '../ErrorContext';

const Notes = () => {
  const { showError } = useError();
  const [notes, setNotes] = useState([]);
  const [selectedNote, setSelectedNote] = useState(null);
  const [newNoteText, setNewNoteText] = useState('');
  const [loadingNotes, setLoadingNotes] = useState(true);
  const [loadingDetail, setLoadingDetail] = useState(false);
  const [addingNote, setAddingNote] = useState(false);

  useEffect(() => {
    const getNotes = async () => {
      setLoadingNotes(true);
      try {
        const response = await fetchNotes();
        if (response.data.status === 'success') {
          setNotes(response.data.message);
        } else {
          showError(response.data.message);
        }
      } catch (error) {
        showError('Failed to fetch notes. Please try again.');
      } finally {
        setLoadingNotes(false);
      }
    };
    getNotes();
  }, [showError]);

  const selectNote = async (id) => {
    setLoadingDetail(true);
    try {
      const response = await fetchNote(id);
      if (response.data.status === 'success') {
        setSelectedNote(response.data.message);
      } else {
        showError(response.data.message);
      }
    } catch (error) {
      showError('Failed to fetch the note. Please try again.');
    } finally {
      setLoadingDetail(false);
    }
  };

  const handleAddNote = async () => {
    setAddingNote(true);
    try {
      const response = await createNote({ text: newNoteText });
      if (response.data.status === 'success') {
        setNotes([...notes, response.data.message]);
        setNewNoteText('');
      } else {
        showError(response.data.message);
      }
    } catch (error) {
      showError('Failed to add the note. Please try again.');
    } finally {
      setAddingNote(false);
    }
  };

  return (
    <Container>
      <Typography variant="h4" component="h2" gutterBottom>
        Notes Overview
      </Typography>
      <Grid container spacing={2}>
        <Grid item xs={12} md={4}>
          <Typography variant="h6">Add New Note</Typography>
          <TextField
            fullWidth
            margin="normal"
            variant="outlined"
            placeholder="Enter note text"
            value={newNoteText}
            onChange={(e) => setNewNoteText(e.target.value)}
            disabled={addingNote}
          />
          <Button
            variant="contained"
            color="primary"
            onClick={handleAddNote}
            disabled={addingNote}
          >
            Add Note
          </Button>

          <Typography variant="h6" component="h3" style={{ marginTop: '20px' }}>
            Notes List
          </Typography>
          {loadingNotes ? (
            <CircularProgress />
          ) : (
            <List>
              {notes.map((note) => (
                <ListItem
                  key={note}
                  button
                  onClick={() => selectNote(note)}
                >
                  <ListItemText primary={note} />
                </ListItem>
              ))}
            </List>
          )}
        </Grid>
        <Grid item xs={12} md={8}>
          {/* Note Detail */}
          {loadingDetail ? (
            <CircularProgress />
          ) : selectedNote ? (
            <>
              <Typography variant="h6"
              component="h3">
                Note Detail
              </Typography>
              <Typography variant="body1">{selectedNote.text}</Typography>
            </>
          ) : (
            <Typography variant="body1">
              Select a note to view its details.
            </Typography>
          )}
        </Grid>
      </Grid>
    </Container>
  );
};

export default Notes;
