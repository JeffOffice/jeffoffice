# Traduzir texto
<!-- keywords: traduzir, tradução, tradutor, idioma, idiomas, espanhol, francês, alemão, italiano, português, neerlandês, russo, ucraniano, indonésio, offline, neste computador, transferir, modelos, substituir, inserir abaixo, controlo de alterações -->

**Rever > Idioma > Traduzir** abre o painel **Traduzir**, à direita. Traduz o seu texto no seu próprio computador. As suas palavras não são enviadas para lado nenhum, e não é necessária ligação à Internet depois de os ficheiros de tradução de um idioma estarem no computador.

## Traduzir algum texto

1. Selecione o texto que pretende traduzir. Se não selecionar nada, o painel usa o parágrafo onde está o cursor.
2. Clique em **Traduzir**, no grupo **Idioma**. O painel mostra o seu texto em **Original**.
3. Em **De**, escolha o idioma em que o texto está escrito. O JeffWrite começa com o idioma que o texto tem marcado. Em **Para**, escolha o idioma pretendido. O botão **Trocar** inverte os dois.
4. Clique em **Traduzir**. A tradução aparece em **Tradução**. As palavras a negrito, itálico e sublinhadas mantêm-se a negrito, itálico e sublinhadas, e uma cor ou um tamanho são mantidos onde as palavras tinham o mesmo aspeto antes.
5. Clique em **Substituir** para colocar a tradução no lugar do seu texto, ou em **Inserir abaixo** para a acrescentar em parágrafos novos abaixo do seu texto. Qualquer uma destas ações é um único passo, por isso **Anular** a desfaz. As palavras traduzidas ficam marcadas com o respetivo idioma, para que a verificação ortográfica as verifique nesse idioma.

Se mover o cursor ou alterar o texto depois de clicar em **Traduzir**, clique em **Usar Seleção** para ler o texto de novo e, depois, em **Traduzir** outra vez. O painel não coloca uma tradução sobre texto que já não é o texto que traduziu.

Quando o **Controlar Alterações** está ativado, **Substituir** e **Inserir abaixo** são alterações registadas, tal como a sua escrita, para que um revisor possa ver o que lá estava antes. As palavras traduzidas assumem então o aspeto que a sua escrita teria. As palavras que uma alteração registada eliminou são deixadas como estão: não são traduzidas nem copiadas por **Inserir abaixo**.

**Cancelar** interrompe uma tradução que demore demasiado, e o mesmo acontece ao fechar o painel: a janela nunca fica à espera dela. Textos longos demoram mais: uma página de texto demora alguns segundos na maioria dos computadores. O painel traduz no máximo 1 000 000 de letras de cada vez (cerca de 200 páginas); se tiver selecionado mais, avisa e pede que selecione menos. Os parágrafos longos são divididos em partes de cerca de 1500 letras, no fim de uma frase sempre que possível, e depois são novamente juntos.

## A primeira vez que usa um idioma

Uma tradução precisa de um conjunto de ficheiros, chamado modelo, para cada sentido: um para inglês-alemão e outro para alemão-inglês. Cada modelo tem cerca de 37 MB. Os ficheiros não fazem parte do JeffWrite. Na primeira vez que traduz com um idioma, o painel indica o que é necessário e o respetivo tamanho, por exemplo "Transferir alemão (74 MB)?". Nada é transferido até clicar nesse botão. O painel mostra o progresso da transferência, e **Cancelar** interrompe-a sem deixar nada para trás.

O JeffWrite verifica cada ficheiro através de uma impressão digital (uma soma de verificação SHA-256) antes de o utilizar. Um ficheiro que não corresponda é eliminado, e o idioma não fica instalado.

Os ficheiros são guardados numa pasta que todas as aplicações JeffOffice partilham (uma pasta chamada JeffOffice na pasta de dados do seu utilizador, em Translation/models), por isso um idioma transferido numa aplicação fica pronto nas outras, e é transferido apenas uma vez. Dois idiomas que não têm modelo próprio, como alemão e francês, passam pelo inglês, por isso o painel precisa dos dois modelos.

## Quando um idioma não tem modelo

Se um par de idiomas não tiver modelo, o painel avisa e oferece **Abrir no Navegador, em Alternativa**. Isso abre um site de tradução gratuito, o LibreTranslate (ou o serviço cujo endereço escolheu na definição respetiva), com o seu texto. Só nesse caso o texto sai do seu computador, porque é enviado para esse site, e o painel deixa de indicar que o texto fica no computador. Nada é enviado até clicar nesse botão.

## Sobre a qualidade

Os modelos são pequenos, por isso funcionam depressa num computador comum. Traduzem bem frases e pior etiquetas curtas: uma palavra isolada ou o nome de um botão podem sair errados. Leia a tradução antes de a utilizar. Nomes, números e tabulações são mantidos como estavam.

Imagens, tabulações e quebras de linha dentro de um parágrafo são deixadas como estão. A tradução preenche as palavras entre elas.

## Ver também

- [Contar palavras, definir o idioma, verificar a acessibilidade e procurar palavras](proofing-tools)
