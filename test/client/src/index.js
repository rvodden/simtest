import React from 'react';
import ReactDom from 'react-dom'

class Led extends React.Component {
    constructor(props) {
        super(props);
        this.state = {
            value: this.props.value,
        }
    }

    handleData(event) {
        this.setState({ value: event.message.value });
    }

    render() {
        return (
            <circle cx={30} cy={50} r={10} fill={this.state.value ? "red" : "black"} stroke="black" />
        )
    }

}

class Button extends React.Component {
    constructor(props) {
        super(props);
        this.sendMessage = this.props.sendMessage
        this.id = this.props.id
    }

    render() {
        return (
            <circle cx={60} cy={50} r={10} fill="grey" stroke="black"
                onPointerDown = { () => this.sendMessage(true)  }
                onPointerUp   = { () => this.sendMessage(false) }
            />
        )
    }
}

class Board extends React.Component {
    constructor(props) {
        super(props)
        this.setLed = element => {
            this.led = element;
        }
    }

    componentDidMount() {
        this.connection = new WebSocket('ws://localhost:7681','websocket-protocol');
        this.connection.onmessage = message => {
            console.log(`Received message: ${JSON.stringify(message.data)}`)
            var event = JSON.parse(message.data);
            if(typeof event.message === 'undefined') {
                console.log("This data was not an event")
            } else {
                console.log(`This data is an event: ${event.name}, ${JSON.stringify(event.message)}`)
                this.led.handleData(event)
            }
        };
    }

    sendMessage(message, id) {
        let jsonMessage = { id: 1, message: {value: message}  };
        console.log(`Sending message: ${JSON.stringify(jsonMessage)}`);
        this.connection.send(JSON.stringify(jsonMessage));
    }

    render() {
        return (
            <div>
                <svg version="1.1" width={100} height={100}>
                    <Led
                        value={true}
                        ref={this.setLed}
                        id="0"
                    />
                    <Button
                        sendMessage={(message) => this.sendMessage(message, this.id)}
                        id="1"
                    />
                </svg>
            </div>
        )
    }
}

ReactDom.render(
    <Board />,
    document.getElementById('root')
);
