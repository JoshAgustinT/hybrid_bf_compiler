from random import randint
import sympy
import inspect


# Generalized function to simulate an arbitrary operation
def simulate_operation(func, *args):
    return func(*args)

# Generalized polynomial expansion of all possible terms (sympy minimized)
def poly_expression(*variables):
    # Create a polynomial of the form (1 + var + var^2) for each variable
    poly = 1
    for var in variables:
        poly *= (1 + var + var**2)
    return poly


# Only keep relevant terms (with degree <= 2)
def get_relevant_terms(v):
    v_expanded = v.expand()
    v_terms = []
    for term, coeff in v_expanded.as_coefficients_dict().items():
        if sympy.total_degree(term) <= 2:
            v_terms.append(term)
    return v_terms


# Compute the closed form of a function with symbolic variables
def compute_closed_form(func):
    # Get the argument names and number of arguments the function expects
    func_signature = inspect.signature(func)
    arg_names = list(func_signature.parameters.keys())
    num_args = len(arg_names)

    # Dynamically create symbolic variables for each argument
    symbols = sympy.symbols(arg_names)  # Use argument names directly as symbols
    
    # Get polynomial and extract relevant terms
    v = poly_expression(*symbols)
    v_terms = get_relevant_terms(v)
    
    # Create coefficient symbols
    coeffs = sympy.symbols(f'c:{len(v_terms)}')
    
    # Final expression with terms and coefficients
    exp = sum(c * t for c, t in zip(coeffs, v_terms))
    
    # Generate random points
    points = [tuple(randint(1, 100) for _ in range(num_args)) for _ in range(len(coeffs))]
    
    results = [] 

    # Solve the equations
    for i, s in enumerate(symbols):
        equations = [exp.subs(dict(zip(symbols, pt))) - simulate_operation(func, *pt)[i] for pt in points]
        sol = sympy.solve(equations, coeffs)
        results.append(f"{s} = {exp.subs(sol)}")
    
        

    output_string = "\n".join(results)
    output_string = output_string.replace('\n', ';')
    
    return output_string  # Return the final output string

# [>expr_simple:-0:-1,1:1,2:1,        >expr_simple:--1:1,0:-1,     <<-]
def sanity_check(p1, p2, p3, p0):
    while(p0 != 0):
        #offset 1
        #p1 += -1*p1 // dont do these until end, or just set the 0+ offset one in this case p1 to 0 at end
        p2 += 1*p1
        p3 += 1*p1
        p1 = 0
        #offset 2
        p1 += 1*p2
        p2 += -1*p2
        p2 = 0
        #offset = 0
        p0 -= 1
    return p1, p2, p3, p0

#[ (>1)A(2,1)  A(1,1)  Z  (>1)  A(-1,1)  Z(<2)(-1)]
# Example function to simulate an operation
def example_function(p1, p2, p3, p0):
    while p0 != 0:
        p3 += p1
        p2 += p1
        p1 = p2
        p2 = 0
        p0 -= 1
    return p1, p2, p3, p0

#           [A(-2,1)Z(<3)Z(>3)]
def hanoi_example(p_2, p_3, p0):
    p_2 += p0
    p0 = 0
    p_3 = 0 
    return p_2, p_3, p0


#[Z(<3)Z(+1)(>1)Z(+1)(>2)] 
def hanoi_example2(p0, p_3, p_2):
    p0 = 0
    p_3 = 0
    p_3 += 1
    p_2 = 0
    p_2 += 1

    return p0, p_3, p_2

#[->>>+<<<<expr_simple:--10:1,0:-1,4:-1,                                >]

def mandle_example(p0, p3, p_11,p_1):
    while(p0 != 0):
        #offset 0
        p0 -= 1
        #offset 3
        p3 += 1
        #offset -1
        p_11 += 1*p_1
        p_1 += -1*p_1
        p3 += -1*p_1
        p_1 = 0
        #offset 0


    return p0, p3, p_11,p_1

# [-<<<<<<expr_simple:-0:-1,4:2,5:1,               >>>>>expr_simple:--5:1,0:-1,             <->+>]
def mandle_example2(p0, p_2, p_6,p_1):
    while(p0 != 0):
        p0 -= 1
        #offset -6
        p_2 += 2*p_6
        p_1 += 1*p_6
        p_6 = 0
        #offset -1
        p_6 += 1*p_1
        p_1 = 0
        #offset -2
        p_2 -= 1
        #offset -1
        p_1 += 1
        #offset 0

    return p0, p_2, p_6,p_1



# Main function using generalized closed form computation
def main():
    result = compute_closed_form(example_function)  # Pass any function
    print(result)  # Print the result
    result = compute_closed_form(hanoi_example)  # Pass any function
    print(result)  # Print the result
    result = compute_closed_form(hanoi_example2)  # Pass any function
    print(result)  # Print the result
    
    result = compute_closed_form(mandle_example)  # Pass any function
    print(result) 
    result = compute_closed_form(mandle_example2)  # Pass any function
    print(result) 
    result = compute_closed_form(sanity_check)  # Pass any function
    print(result) 



# Run the main function
if __name__ == "__main__":
    main()
