# Traduzir texto
<!-- keywords: traduzir, tradução, tradutor, idioma, idiomas, espanhol, francês, alemão, italiano, português, neerlandês, russo, ucraniano, indonésio, offline, neste computador, transferir, modelos, substituir, inserir abaixo -->

**Rever > Idioma > Traduzir** abre o painel **Traduzir**. Traduz o seu texto no próprio computador. As suas palavras não são enviadas para lado nenhum, e não é necessária ligação à Internet depois de os ficheiros de tradução de um idioma estarem no computador.

## Traduzir algum texto

1. Selecione o texto que pretende traduzir. Se não houver nada selecionado, o painel usa o parágrafo onde está o cursor. Numa caixa de texto que esteja selecionada mas não em edição, usa todo o texto da caixa.
2. Clique em **Rever > Idioma > Traduzir**. O painel mostra o seu texto em **Original**.
3. Em **De**, escolha o idioma em que está o seu texto. Em **Para**, escolha o idioma pretendido. O botão **Trocar** inverte os dois.
4. Clique em **Traduzir**. A tradução aparece em **Tradução**. As palavras a negrito, itálico e sublinhadas mantêm-se a negrito, itálico e sublinhadas.
5. Clique em **Substituir** para colocar a tradução no lugar do seu texto, ou em **Inserir Abaixo** para a adicionar em novos parágrafos por baixo do seu texto. Qualquer uma destas ações é um único passo, por isso **Anular** a desfaz.

Se mover o cursor ou alterar o texto depois de clicar em **Traduzir**, clique em **Usar Seleção** para ler o texto novamente e, em seguida, clique novamente em **Traduzir**. O painel não coloca uma tradução por cima de texto que já não seja o texto que foi traduzido.

**Cancelar** interrompe uma tradução que esteja a demorar demasiado, e o mesmo acontece ao fechar o painel: a janela nunca fica à espera dela. Textos longos demoram mais: uma página de texto demora alguns segundos na maioria dos computadores. O painel traduz no máximo 1 000 000 de letras de cada vez (cerca de 200 páginas); se houver mais texto selecionado, o painel avisa e pede que selecione menos. Divide parágrafos longos em partes de cerca de 1500 letras, no fim de uma frase sempre que possível, e volta a juntá-las.

## A primeira vez que usa um idioma

Uma tradução precisa de um conjunto de ficheiros, chamado modelo, para cada sentido: um para inglês para alemão, outro para alemão para inglês. Cada modelo tem cerca de 37 MB. Os ficheiros não fazem parte do JeffPub. Na primeira vez que traduzir com um idioma, o painel indica o que é necessário e o respetivo tamanho, por exemplo "Transferir alemão (74 MB)?". Nada é transferido até clicar nesse botão. O painel mostra o progresso da transferência, e **Cancelar** interrompe-a sem deixar nada para trás.

O JeffPub verifica cada ficheiro através de uma impressão digital (uma soma de controlo SHA-256) antes de o utilizar. Um ficheiro que não corresponda é eliminado, e o idioma não fica instalado.

Os ficheiros são guardados numa única pasta partilhada por todas as aplicações JeffOffice (uma pasta chamada JeffOffice na pasta de dados do seu utilizador, em Translation/models), por isso um idioma transferido numa aplicação fica disponível nas outras, e só é transferido uma vez. Dois idiomas que não têm modelo próprio, como alemão e francês, passam pelo inglês, pelo que o painel precisa dos dois modelos.

## Quando um idioma não tem modelo

Se um par de idiomas não tiver modelo, o painel indica-o e oferece o botão **Abrir no Navegador, em Alternativa**. Esse botão abre um site de tradução gratuito, o LibreTranslate (ou o serviço cujo endereço escolheu na definição respetiva), com o seu texto. Só nesse caso o seu texto sai do computador, porque é enviado para esse site, e o painel deixa de indicar que o texto fica no computador. Nada é enviado até clicar nesse botão.

## Sobre a qualidade

Os modelos são pequenos, por isso funcionam rapidamente num computador comum. Traduzem bem frases e pior etiquetas curtas: uma palavra isolada ou o nome de um botão podem sair incorretos. Leia a tradução antes de a utilizar. Nomes, números e tabulações são mantidos como estavam.

As imagens, as tabulações e as quebras de linha dentro de um parágrafo não são alteradas. A tradução preenche apenas as palavras entre elas.

## Relacionados

- [Ortografia, dicionário de sinónimos, pesquisa e hifenização](proofing)
