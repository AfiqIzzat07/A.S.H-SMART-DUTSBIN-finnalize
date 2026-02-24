// Replace with your Firebase config
const firebaseConfig = {
  apiKey: "AIzaSyBD990zggrFpi5Z2DwAOLShTjJUDN8ydBo",
  authDomain: "smart-dustbin-150307.firebaseapp.com",
  databaseURL: "https://smart-dustbin-150307-default-rtdb.asia-southeast1.firebasedatabase.app/",
  projectId: "smart-dustbin-150307",
  storageBucket: "smart-dustbin-150307.firebasestorage.app",
  messagingSenderId: "470186830441",
  appId: "1:470186830441:web:9355f4e1c10e498b26d0f8"
};

firebase.initializeApp(firebaseConfig);
const database = firebase.database();
