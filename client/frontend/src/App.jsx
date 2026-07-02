import React, { useState, useEffect, useRef } from 'react';
import { Send, Hash, MessageSquare, Activity, Wifi, WifiOff, Menu, X, Users } from 'lucide-react';
import './index.css';

const WS_URL = `ws://${window.location.hostname}:8081`;
const ROOMS = ['General', 'Gaming', 'Coding', 'Random'];

function App() {
  const [ws, setWs] = useState(null);
  const [connected, setConnected] = useState(false);
  const [activeRoom, setActiveRoom] = useState('General');
  const [messages, setMessages] = useState({});
  const [roomCounts, setRoomCounts] = useState({});
  const [input, setInput] = useState('');
  const [sidebarOpen, setSidebarOpen] = useState(false);
  
  const messagesEndRef = useRef(null);

  useEffect(() => {
    // Initialize messages state for all rooms
    const initialMessages = {};
    ROOMS.forEach(r => initialMessages[r] = []);
    setMessages(initialMessages);

    // Connect WebSocket
    const socket = new WebSocket(WS_URL);

    socket.onopen = () => {
      setConnected(true);
      setWs(socket);
      // Subscribe to initial room
      socket.send(JSON.stringify({ action: 'subscribe', room: 'General' }));
    };

    socket.onmessage = (event) => {
      try {
        const data = JSON.parse(event.data);
        if (data.type === 'user_count') {
          setRoomCounts(prev => ({ ...prev, [data.room]: data.count }));
        } else if (data.message && data.room) {
          setMessages(prev => ({
            ...prev,
            [data.room]: [...(prev[data.room] || []), { text: data.message, mine: false, id: Date.now() + Math.random() }]
          }));
        }
      } catch (e) {
        console.error('Invalid JSON received:', event.data);
      }
    };

    socket.onclose = () => {
      setConnected(false);
      setWs(null);
    };

    return () => {
      socket.close();
    };
  }, []);

  useEffect(() => {
    messagesEndRef.current?.scrollIntoView({ behavior: 'smooth' });
  }, [messages, activeRoom]);

  const joinRoom = (roomName) => {
    if (ws && connected && roomName !== activeRoom) {
      ws.send(JSON.stringify({ action: 'subscribe', room: roomName }));
      setActiveRoom(roomName);
      setSidebarOpen(false);
    }
  };

  const sendMessage = (e) => {
    e.preventDefault();
    if (!input.trim() || !ws || !connected) return;

    // Send to backend
    ws.send(JSON.stringify({ action: 'publish', room: activeRoom, message: input }));
    
    // Add locally immediately (optimistic update)
    setMessages(prev => ({
      ...prev,
      [activeRoom]: [...(prev[activeRoom] || []), { text: input, mine: true, id: Date.now() + Math.random() }]
    }));
    
    setInput('');
  };

  const currentMessages = messages[activeRoom] || [];

  return (
    <div className="app-container">
      {/* Sidebar */}
      <div className={`sidebar ${sidebarOpen ? 'open' : ''}`}>
        <div className="sidebar-header">
          <h2><Activity size={24} color="var(--primary)" /> NexusChat</h2>
          <button className="mobile-toggle close" onClick={() => setSidebarOpen(false)}>
            <X size={20} />
          </button>
        </div>
        <div className="room-list">
          {ROOMS.map(room => (
            <div 
              key={room} 
              className={`room-item ${activeRoom === room ? 'active' : ''}`}
              onClick={() => joinRoom(room)}
            >
              <div style={{ display: 'flex', alignItems: 'center', gap: '8px', flex: 1 }}>
                <Hash size={18} />
                {room}
              </div>
              {roomCounts[room] > 0 && (
                <span style={{ fontSize: '0.75rem', background: 'rgba(255,255,255,0.1)', padding: '2px 6px', borderRadius: '10px' }}>
                  {roomCounts[room]}
                </span>
              )}
            </div>
          ))}
        </div>
      </div>

      <div className="sidebar-overlay" onClick={() => setSidebarOpen(false)}></div>

      {/* Main Chat */}
      <div className="chat-area">
        <div className="chat-header">
          <div style={{ display: 'flex', alignItems: 'center', gap: '10px' }}>
            <button className="mobile-toggle menu" onClick={() => setSidebarOpen(true)}>
              <Menu size={20} />
            </button>
            <h3># {activeRoom}</h3>
            {roomCounts[activeRoom] > 0 && (
              <span style={{ display: 'flex', alignItems: 'center', gap: '4px', fontSize: '0.8rem', color: 'var(--text-muted)', marginLeft: '8px' }}>
                <Users size={14} />
                {roomCounts[activeRoom]} Active
              </span>
            )}
          </div>
          <div className={`status-badge ${connected ? '' : 'disconnected'}`}>
            {connected ? (
              <span style={{ display: 'flex', alignItems: 'center', gap: '6px' }}><Wifi size={14} /> Connected</span>
            ) : (
              <span style={{ display: 'flex', alignItems: 'center', gap: '6px' }}><WifiOff size={14} /> Offline</span>
            )}
          </div>
        </div>

        <div className="messages-container">
          {currentMessages.length === 0 ? (
            <div style={{ margin: 'auto', color: 'var(--text-muted)', display: 'flex', flexDirection: 'column', alignItems: 'center', gap: '12px' }}>
              <MessageSquare size={48} opacity={0.5} />
              <p>No messages in #{activeRoom} yet. Say hello!</p>
            </div>
          ) : (
            currentMessages.map((msg) => (
              <div key={msg.id} className={`message-wrapper ${msg.mine ? 'mine' : 'theirs'}`}>
                <div className="message-bubble">
                  {msg.text}
                </div>
              </div>
            ))
          )}
          <div ref={messagesEndRef} />
        </div>

        <div className="input-area">
          <form className="input-form" onSubmit={sendMessage}>
            <input 
              type="text" 
              className="message-input" 
              placeholder={`Message #${activeRoom}...`}
              value={input}
              onChange={(e) => setInput(e.target.value)}
              disabled={!connected}
            />
            <button type="submit" className="send-button" disabled={!connected || !input.trim()}>
              <Send size={20} />
            </button>
          </form>
        </div>
      </div>
    </div>
  );
}

export default App;
