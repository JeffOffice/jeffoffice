# Transições entre slides
<!-- keywords: transição, transições, transformar, morph, esmaecer, empurrar, revelar, dividir, cobrir, descobrir, zoom, cortar, corte, opções de efeito, objetos, palavras, caracteres, correspondência, par, nome, direção, duração, avançar slide, ao clicar com o mouse, após, automático, intervalo, tempo, aplicar a todos, visualizar, estrela, efeito de passagem -->

Uma **transição** é o efeito reproduzido quando a apresentação de slides passa de um slide para o próximo. A transição pertence ao slide em que ela entra. A apresentação de slides a reproduz ao avançar para esse slide.

## Adicionar uma transição

1. Clique no slide no painel de slides.
2. Clique na guia **Transições**. Na galeria **Transição para Este Slide**, clique em um efeito: **Nenhuma**, **Transformar**, **Esmaecer**, **Empurrar**, **Revelar Gradualmente**, **Dividir**, **Revelar**, **Cobrir**, **Descobrir**, **Zoom** ou **Cortar**. A miniatura do slide no painel de slides ganha uma pequena estrela para indicar que ele tem uma transição.
3. Clique em **Transições > Visualizar > Visualizar** para vê-la reproduzida na entrada do slide, sobre a área do slide. Pressione Esc para interrompê-la.

Escolha **Nenhuma** para remover uma transição.

## Escolher como ela acontece

Clique em **Opções de Efeito** para ver as opções do efeito:

- **Empurrar**, **Revelar Gradualmente**, **Revelar**, **Cobrir** e **Descobrir** têm uma direção: **Da Direita**, **Da Esquerda**, **De Baixo** ou **De Cima**.
- **Dividir** e **Zoom** vão **Para Fora** ou **Para Dentro**.
- **Transformar** move **Objetos**, **Palavras** ou **Caracteres**. Veja abaixo.
- **Esmaecer** e **Cortar** não têm opções. Um corte passa por uma tela preta.

## Transformar

**Transformar** move o que dois slides têm em comum. Um **objeto** é um item em um slide: uma forma, uma imagem, uma caixa de texto, uma linha ou uma tabela. A apresentação passa de um slide para o próximo. Cada objeto que está nos dois slides desliza da posição em que está no primeiro slide para a posição em que está no segundo. No caminho, ele aumenta ou diminui, gira e muda de cor. Um objeto que está somente no primeiro slide desaparece aos poucos. Um objeto que está somente no segundo slide aparece aos poucos.

Para criar uma transformação:

1. Clique com o botão direito do mouse em um slide no painel de slides e escolha **Duplicar Slide**. A cópia tem os mesmos objetos.
2. Na cópia, mova um objeto, aumente-o, gire-o ou altere a cor dele. Você também pode alterar o texto dele ou adicionar e excluir objetos.
3. Clique na cópia e depois em **Transições > Transição para Este Slide > Transformar**. Uma transformação leva 2 segundos. Você pode alterar isso em **Duração**.
4. Clique em **Visualizar** para assistir a ela ou inicie a apresentação de slides.

### Quais objetos formam pares

O JeffDeck forma pares entre um objeto do primeiro slide e um objeto do segundo slide. Ele tenta estes critérios, nesta ordem. Cada objeto entra em no máximo um par.

1. **Um nome que começa com !!.** Dois objetos com o mesmo nome, quando o nome começa com dois pontos de exclamação, sempre formam um par. Podem ser quaisquer dois objetos, até mesmo um círculo e uma estrela.
2. **O mesmo nome.** Dois objetos do mesmo tipo com o mesmo nome formam um par. Uma caixa de texto e uma forma são tipos diferentes.
3. **O mesmo texto.** Duas caixas de texto com as mesmas palavras formam um par, sejam quais forem os nomes delas. O mesmo vale para duas formas com as mesmas palavras.
4. **A mesma imagem.** Duas imagens da mesma figura formam um par, mesmo que tenham outro nome ou outro tamanho.
5. **O mesmo tipo de forma.** Dois círculos formam um par. O mesmo vale para duas linhas ou duas tabelas.

Quando vários objetos podem formar um par, os que estão mais próximos entre si são agrupados primeiro.

Os nomes dos objetos vêm do arquivo da apresentação. Uma apresentação criada em outro programa mantém os nomes que deu aos objetos, e o JeffDeck os usa.

### Como um par se move

- Dois objetos com a mesma forma são um único objeto que se move. A posição, o tamanho, a rotação, a cor de preenchimento, a cor da linha e a transparência mudam aos poucos. O corte de uma imagem também muda.
- Dois objetos que não têm a mesma forma, como um círculo e um quadrado, vão para as mesmas posições enquanto um se transforma gradualmente no outro.
- Se o texto de um par não for o mesmo, o texto antigo se transforma gradualmente no novo.
- Objetos que uma animação oculta ficam de fora. Um objeto que uma animação moveu parte da posição em que terminou.

### O que se move: Objetos, Palavras ou Caracteres

Clique em **Opções de Efeito** para escolher:

- **Objetos.** O texto de um par permanece no objeto. O texto antigo se transforma gradualmente no novo.
- **Palavras.** Uma palavra que existe nos dois textos se move da posição antiga para a nova. As outras palavras desaparecem e aparecem aos poucos. Um sinal como a vírgula conta como uma palavra à parte.
- **Caracteres.** O mesmo, uma letra ou um número de cada vez.

**Palavras** e **Caracteres** funcionam em caixas de texto e formas. Textos com mais de 16.000 caracteres, textos na vertical e textos em uma forma invertida são reproduzidos como **Objetos**. O mesmo acontece com um par que não tem nenhuma palavra ou caractere em comum.

A transformação também é uma transição em outros programas de apresentação (nas versões 2019 e mais recentes). O JeffDeck a lê e salva da mesma forma que eles. Programas mais antigos reproduzem uma transformação como um esmaecimento.

## Definir o intervalo

O grupo **Intervalo** define quanto tempo a transição leva e como a apresentação sai do slide.

- **Duração** é quantos segundos o efeito leva. Uma nova transição leva 0,7 segundo. Você pode digitar de 0,01 a 59.
- **Ao Clicar com o Mouse** vem ativado por padrão. A apresentação sai deste slide quando o apresentador clica. Desative essa opção para avançar com uma tecla, como Espaço ou Seta para a Direita. As animações do slide continuam sendo reproduzidas com um clique.
- **Após** faz a apresentação avançar sozinha depois do número de segundos que você digitar. Você pode usar **Após** junto com **Ao Clicar com o Mouse**: a apresentação avança com o que acontecer primeiro.

**Aplicar a Todos** aplica a transição e o intervalo do slide atual a todos os slides.

Cada uma dessas ações é uma etapa que Ctrl+Z desfaz.

## Transições de outros arquivos

Uma apresentação criada em outro programa pode ter uma transição para a qual o JeffDeck não tem efeito. Ela é reproduzida como um esmaecimento e salva como estava, para continuar funcionando no programa de origem.

## Relacionados

- [Guia Transições](tab-transitions)
- [Animações](animations)
- [Reproduzir a apresentação de slides e usar o Modo de Exibição do Apresentador](show)
