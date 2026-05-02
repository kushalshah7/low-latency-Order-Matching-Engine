import React, { useState } from 'react';

function App() {
  const [demoOutput, setDemoOutput] = useState('');
  const [error, setError] = useState('');

  const runDemo = async () => {
    setError('');
    setDemoOutput('Running...');
    try {
      const res = await fetch('/api/run_demo');
      const data = await res.json();
      setDemoOutput(data.output || '');
      if (data.error) setError(data.error);
    } catch (e) {
      setError(e.toString());
    }
  };

  return (
    <div style={{ padding: 20 }}>
      <h1>Low-Latency Order Matching Engine</h1>
      <button onClick={runDemo}>Run Demo</button>
      <pre style={{ background: '#eee', padding: 10, marginTop: 20 }}>{demoOutput}</pre>
      {error && <div style={{ color: 'red' }}>Error: {error}</div>}
    </div>
  );
}

export default App;
