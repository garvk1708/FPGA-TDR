require('dotenv').config();
const express = require('express');
const cors = require('cors');
const { GoogleGenAI } = require('@google/genai');

const app = express();
const port = 3000;

app.use(cors());
app.use(express.json());
app.use(express.static(__dirname)); // To serve the HTML file

console.log("STARTUP: GEMINI_API_KEY is", process.env.GEMINI_API_KEY ? "SET (" + process.env.GEMINI_API_KEY.substring(0, 5) + "...)" : "UNDEFINED");

// In-memory distance storage
let currentDistance = 2.4; // Default starting value

// --- ESP8266 Endpoints ---
// GET is easier for some simple ESP8266 setups, but POST is also provided.
app.get('/api/update-distance', (req, res) => {
    const { dist } = req.query;
    if (dist && !isNaN(parseFloat(dist))) {
        currentDistance = parseFloat(dist);
        return res.json({ success: true, distance: currentDistance });
    }
    return res.status(400).json({ error: 'Invalid distance provided' });
});

app.post('/api/update-distance', (req, res) => {
    const { dist } = req.body;
    if (dist && !isNaN(parseFloat(dist))) {
        currentDistance = parseFloat(dist);
        return res.json({ success: true, distance: currentDistance });
    }
    return res.status(400).json({ error: 'Invalid distance provided' });
});

// --- Frontend Endpoints ---
app.get('/api/distance', (req, res) => {
    res.json({ distance: currentDistance });
});

// --- Gemini API Endpoint ---
app.post('/api/predict-fault', async (req, res) => {
    const { cableAge, signalVelocity } = req.body;

    if (!cableAge || !signalVelocity) {
        return res.status(400).json({ error: 'Missing cableAge or signalVelocity' });
    }

    if (!process.env.GEMINI_API_KEY || process.env.GEMINI_API_KEY === 'YOUR_API_KEY_HERE') {
        return res.status(500).json({
            error: 'Gemini API key is not configured. Please add it to the .env file.'
        });
    }

    try {
        const ai = new GoogleGenAI({ apiKey: process.env.GEMINI_API_KEY });
        const prompt = `You are an expert cable diagnostics AI. 
I have a cable fault detected by a Time Domain Reflectometer (TDR).
Cable characteristics:
- Age: ${cableAge} years
- Signal Velocity: ${signalVelocity}

Based on these parameters, predict the likely type of physical damage or degradation causing the fault.
Keep the explanation brief, technical, and precise (under 3 sentences).`;

        const response = await ai.models.generateContent({
            model: 'gemini-2.5-flash',
            contents: prompt,
        });

        res.json({ prediction: response.text });
    } catch (error) {
        console.error('Gemini Error:', error);
        res.status(500).json({ error: 'Failed to generate prediction. Please check server logs.' });
    }
});

app.listen(port, () => {
    console.log(`Server running at http://localhost:${port}`);
    console.log(`ESP8266 Update Endpoint: GET or POST http://localhost:${port}/api/update-distance?dist=VALUE`);
});
