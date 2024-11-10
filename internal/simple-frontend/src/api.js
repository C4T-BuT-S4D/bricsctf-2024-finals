import axios from 'axios';

const API = axios.create({
	baseURL: 'http://localhost:2112/api',
});

// Request interceptor to add the token to headers
API.interceptors.request.use((config) => {
  const token = localStorage.getItem('token');
  console.log("AAAAAAAAAAAAAAAAAAAA", token);
  if (token) {
    config.headers.Authorization = `Bearer ${token}`;
  }
  console.log("BBBBBBBBBBBBBBBB", config.headers.Authorization);
  return config;
});

export const register = (user) => API.post('/register', user);
export const login = (user) => API.post('/login', user);
export const fetchNotes = () => API.get('/notes');
export const fetchNote = (id) => API.get(`/note/${id}`);
export const createNote = (note) => API.put('/note', note);

export default API;
