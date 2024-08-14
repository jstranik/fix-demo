class Logic:
    def should_trade(self, symbol:str, price:float) -> bool:
        print(f"Analyzing {symbol} @ {price} ... ", end = "")
        result = price > 15
        print("trade" if result else "skip")
        return result
